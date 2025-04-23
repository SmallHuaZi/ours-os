#include <ours/config.hpp>
#include <ours/mem/pm_node.hpp>

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/memory_model.hpp>

#include <ours/panic.hpp>
#include <ours/assert.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/util/enum_sequence.hpp>
#include <ustl/function/bind.hpp>

#include <logz4/log.hpp>

namespace ours::mem {
    auto ZoneQueues::init_queues() -> void {
        auto offset = 0;
        ZoneQueue helper = queue_storage_; 
        queues_[LocalSequential] = helper.subspan(0 * NR_ZONES_PER_NODE, NR_ZONES_PER_NODE);
        queues_[LocalContiguous] = helper.subspan(1 * NR_ZONES_PER_NODE, NR_ZONES_PER_NODE);
        queues_[SharedAffinity] = helper.subspan(2 * NR_ZONES_PER_NODE, MAX_ZONES);
        queues_[SharedSequential] = helper.subspan(2 * NR_ZONES_PER_NODE + MAX_ZONES, MAX_ZONES);
    }

    template <>
    FORCE_INLINE
    auto ZoneQueues::insert_remote_queue<ZoneQueues::QueueType::SharedAffinity>(ZoneQueue const &remote_queue)
        -> void {
        for (auto &i: remote_queue) {
            push_back(i, QueueType::SharedAffinity);
        }
    }

    template <>
    FORCE_INLINE
    auto ZoneQueues::insert_remote_queue<ZoneQueues::QueueType::SharedSequential>(ZoneQueue const &queue) 
        -> void {
    }

    // Automatically dispatch insertion logic among different queues.
    FORCE_INLINE
    auto ZoneQueues::insert_remote_queue(ZoneQueue const &queue) -> void {
        [&] <QueueType... Types> (ustl::EnumSequence<QueueType, Types...>) {
            (insert_remote_queue<Types>(queue), ...);
        } (ustl::MakeEnumSequenceT<QueueType, LocalContiguous + 1, MaxNumQueues>());
    }

    auto ZoneQueues::connect(NodeMask const &nodemask) -> void {
        NodeRoute node_path;
        node_path.clear();

        PmNode::build_optimal_node_route(nid_, node_path, connected_);

        for (auto next : node_path) {
            auto zq = PmNode::node(next)->zone_queues();
            insert_remote_queue(zq->get_queue(QueueType::LocalContiguous));
        }
    }

    static auto to_string(ZoneQueues::QueueType type) -> char const * {
        switch (type) {
            case ZoneQueues::LocalSequential:   return "l-seq";
            case ZoneQueues::LocalContiguous:   return "l-contig";
            case ZoneQueues::SharedAffinity:    return "s-affi";
            case ZoneQueues::SharedSequential:  return "s-seq";
        }

        return "Unknown";
    }

    auto ZoneQueues::dump_queue(QueueType type) const -> void {
        log::debug("Node[{}-{}]", nid_, to_string(type));

        auto const queue = get_queue(type);
        for (auto zone: queue) {
            if (zone) {
                log::debug("    None-{}| Zone-{:<6}| [0x{:X}, 0x{:X})", 
                    zone->which_node(), 
                    to_string(zone->zone_type()),
                    zone->start_pfn(),
                    zone->end_pfn()
                );
            } else {
                log::debug("    NULL");
            }
        }
    }

    struct ZoneIterator {
        typedef ZoneIterator     Self;
        typedef ZoneQueues::ZoneRef             ZoneRef;
        typedef ZoneQueues::ZoneQueue::IterMut  Inner;

        auto set_nodes(NodeMask const &nm) -> Self & {
            nodes = &nm;
            return *this;
        }

        auto set_zone_range(ZoneType lower, ZoneType upper) -> Self & {
            lower_zone_type = lower;
            upper_zone_type = upper;
            return *this;
        }

        auto set_zone_iterator(Inner begin, Inner end) -> Self & {
            first = begin;
            last = end;
            return *this;
        }

        auto move_next() -> ZoneRef;

        Inner first;
        Inner last;
        NodeMask const *nodes;
        ZoneType lower_zone_type;   // Reserved and not used now.
        ZoneType upper_zone_type;
    };

    auto ZoneIterator::move_next() -> ZoneRef {
        while (first != last) {
            auto zref = (*first);
            if (zref->zone_type() > upper_zone_type) {
                ++first;
            }

            if (!nodes->test(zref->which_node())) {
                ++first;
            }

            // Advance for next iteration.
            return *(first++);
        }

        return ZoneRef();
    }

    struct AllocationContext {
        typedef AllocationContext   Self;

        auto build(PmNode *node, Gaf gaf, usize order, NodeMask const &nodemask) 
            -> Status {
            ziter.set_nodes(nodemask)
                 .set_zone_range(gaf_zone_type(gaf), gaf_zone_type(gaf));
            
            ZoneQueues::QueueType queue_type;
            if (bool(gaf & Gaf::OnlyThisNode)) {
                queue_type = ZoneQueues::LocalContiguous;
            } else {
                queue_type = ZoneQueues::SharedAffinity;
            }
            auto zq = node->zone_queues()->get_queue(queue_type);
            ziter.set_zone_iterator(zq.begin(), zq.end());
            
            return Status::Ok;
        }

        ZoneIterator ziter;
    };

    static Gaf g_gaf_allowed = kGafBoot;

    PmNode::PmNode(NodeId nid)
        : id_(nid),
          page_queues_(),
          zone_queues_(nid)
    {
        DEBUG_ASSERT(!s_node_list[nid]);
        s_node_list[id_] = this;
        s_num_nodes += 1;
    }

    auto PmNode::init(Pfn start, Pfn end) -> Status {
        canary_.verify();

        start_pfn_ = start;
        spanned_frames_ = end - start;
        present_frames_ = 0;

        if (spanned_frames_ != 0) {
            set_node_state(id_, NodeStates::Memory, true);
        }

        set_node_online(id_, true);
        return Status::Ok;
    }

    auto PmNode::build_optimal_node_route(NodeId from, NodeRoute &path, NodeMask const &filter) -> void {
        NodeMask visited;
        visited.set(from, true);

        auto find_next = [&] () {
            NodeId result = MAX_NODE;
            usize distance = ustl::NumericLimits<usize>::max();
            global_node_states().for_each_state(NodeStates::Memory, [&] (NodeId to) {
                if (visited.test(to) || filter.test(to)) {
                    return;
                }

                auto const this_dis = PmNode::distance(from, to);
                if (distance > this_dis) {
                    result = to;
                    distance = this_dis;
                }
            });

            if (result != MAX_NODE) {
                visited.set(result);
            }

            return result;
        };

        while (1) {
            auto next = find_next();
            if (next == MAX_NODE) {
                break;
            }

            path.set_next(next);
        }
    }

    auto PmNode::alloc_frame_core(Gaf gaf, usize order, AllocationContext &context) -> PmFrame * {
        PmFrame *frame = nullptr;
        while (auto zref = context.ziter.move_next()) {
            // Here we can do some coloring work to tag node. The mark behaviour itself may be a time of
            // penalty or reward to a node.
            frame = zref->alloc_frame(gaf, order);
            if (frame) {
                break;
            }
        }

        return frame;
    }

    auto PmNode::finish_allocation(PmFrame *frame, Gaf gaf, usize order, AllocationContext const &context) -> void {

    }

    auto PmNode::alloc_frame(Gaf gaf, usize order, NodeMask const &nodes) -> ustl::Result<PmFrame *, Status> {
        if (order > MAX_FRAME_ORDER) {
            return ustl::err(Status::InvalidArguments);
        }
        gaf &= g_gaf_allowed;

        AllocationContext context;
        auto status = context.build(this, gaf, order, nodes);
        if (Status::Ok != status) {
            return ustl::err(status); 
        }

        // First attempt to allocate.
        PmFrame *result = alloc_frame_core(gaf, order, context);
        if (!result) {
            // Now no any way to reclaim frames, so directly panic().
            panic("No enough frames for request");
        }

        finish_allocation(result, gaf, order, context);
        return ustl::ok(result);
    }

    auto PmNode::alloc_frames(Gaf flags, usize n, ai_out FrameList<> *list, NodeMask const &mask) -> Status {
        DEBUG_ASSERT(list != nullptr, "");
        for (usize order = 0; n > 0; order += 1) {
            if ((n & 1) == 0) {
                continue;
            }

            if (auto frame = this->alloc_frame(flags, order, mask)) {
                list->push_back(*frame.unwrap());
                n >>= 1;
            } else {
                free_frames(list);
                return frame.unwrap_err();
            }
        }

        return Status::Ok;
    }

    auto PmNode::alloc_frames_bulk(Gaf flags, usize n, ai_out FrameList<> *list) -> Status {
        DEBUG_ASSERT(list != nullptr, "");
        while (n--) {
            if (auto frame = this->alloc_frame(flags, 0)) {
                list->push_back(*frame.unwrap());
                n >>= 1;
            } else {
                free_frames(list);
                return frame.unwrap_err();
            }
        }

        return Status::Ok;
    }

    auto PmNode::free_frame(PmFrame *frame, usize order) -> void {
        DEBUG_ASSERT(frame, "");
        auto const node = PmNode::node(frame->nid());
        auto const zone = node->zone_queues_.get_local_zone(frame->zone());
        DEBUG_ASSERT(zone, "");

        zone->free_frame(frame, order);
    }

    auto PmNode::free_frames(FrameList<> *list) -> void {
        list->clear_and_dispose([] (PmFrame *frame) {
            free_frame(frame, frame->nid());
        });
    }

    auto PmNode::dump() const -> void {
        // TODO(SmallHuaZi) merge them to a log sentence.
        log::info("Node[{}]: ", id_);
        log::info("  start pfn: {}", start_pfn_);
        log::info("  spanned frames: {}", spanned_frames_.load());
        log::info("  present frames: {}", present_frames_.load());
        log::info("  reserved frames: {}", reserved_frames_.load());

        for (auto zone : zone_queues_.get_queue(ZoneQueues::LocalContiguous)) {
            log::info("  Zone-{}:", to_string(zone->zone_type()));
            log::info("    start pfn: {}", zone->start_pfn());
            log::info("    spanned frames: {}", zone->spanned_frames());
            log::info("    present frames: {}", zone->present_frames());
            log::info("    reserved frames: {}", zone->reserved_frames());
            log::info("    managed frames: {}", zone->managed_frames());
        }
    }

} // namespace ours::mem