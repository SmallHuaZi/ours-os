#include <ours/config.hpp>
#include <ours/mem/pm_node.hpp>

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/new.hpp>

#include <ours/panic.hpp>
#include <ours/assert.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/algorithms/minmax.hpp>

using ustl::mem::construct_at;
using ustl::algorithms::clamp;

namespace ours::mem {
    NodeStates::Inner       NodeStates::NODE_STATES;

    template <typename Inner>
    struct ZoneQueues::Iterator
    {
        typedef Iterator    Self; 

        auto init(Gaf gaf)
        {

        }

        auto move_next() -> ZoneRef *
        {
            while (first != last) {
                
            }

            return 0;
        }

        Inner first;
        Inner last;
        NodeMask *nodes;
        ZoneType highest_zone_type;
    };

    FORCE_INLINE
    ZoneQueues::ZoneQueues(NodeId nid)
        : nid_(nid)
    {}

    auto ZoneQueues::emplace(PmZone *zone, ZoneType ztype) -> void 
    {
        DEBUG_ASSERT(ztype < NR_ZONES_PER_NODE);
        DEBUG_ASSERT(local_zones_[ztype] == 0);

        if (!zone) {
            panic("Error {}", 0);
            return;
        }

        local_zones_[ztype] = zone;
        for (auto i = 0; i < local_zones_.size(); ++i) {
            if (ztype > i) {
                local_queue_.emplace(local_queue_.begin() + i, zone, ztype);
            }
        }

        NodeStates::set_state(nid_, NodeStates::NodeStateType(ztype));

        this->insert_queue(local_queue_);
    }

    FORCE_INLINE
    auto ZoneQueues::iter(QueueType type) -> IterMut
    {  return {}; }

    auto ZoneQueues::rev_iter(QueueType type) -> RevIterMut
    {}

    template <>
    FORCE_INLINE
    auto ZoneQueues::priv_insert_queue<ZoneQueues::NodeAffinity>(LocalQueue const &queue) -> void
    {
        // auto const nid = zone->which_node();
        // auto &queue = this->node_affinity_queue_;
        // auto const n = queue.size();

        // // Find the index range of zone in node[nid]
        // for (auto i = 0; i < n; ++i) {
        //     auto other_nid = queue[i].zone->which_node();
        //     auto const x = PmNode::distance(this->nid_, nid);
        //     auto const y = PmNode::distance(this->nid_, other_nid);
        //     if (x <= y) {
        //         break;
        //     }
        // }
    }

    template <>
    FORCE_INLINE
    auto ZoneQueues::priv_insert_queue<ZoneQueues::ZonePriority>(LocalQueue const &queue) -> void
    {
        // auto const nid = zone->which_node();
        // auto &queue = this->zone_priority_queue_[ztype];
        // auto const n = queue.size();

        // for (auto i = 0; i < n; ++i) {
        //     auto other_nid = queue[i]->which_node();
        //     auto const x = PmNode::distance(this->nid_, nid);
        //     auto const y = PmNode::distance(this->nid_, other_nid);
        //     if (x < y) {
        //         queue.insert(queue.begin() + i, zone);
        //         return;
        //     }
        // }

        // queue.push_back(zone);
    }

    auto ZoneQueues::insert_queue(LocalQueue const &queue) -> void
    {
        this->priv_insert_queue<NodeAffinity>(queue);
        this->priv_insert_queue<ZonePriority>(queue);
    }

    struct AllocContext
    {
        AllocContext(ZoneQueues::IterMut iterator, usize order, NodeMask NodeMask)
            : order(order),
              NodeMask(NodeMask),
              iterator(iterator)
        {}

        usize order;
        NodeMask NodeMask;
        ZoneQueues::IterMut iterator;
    };

    CXX11_CONSTEXPR
    NodeMask const DEFAULT_ONLINE_NODE_MASK = {  };

    static Gaf GAF_ALLOWED = GAF_BOOT;

    PmNode::PmNode(NodeId nid)
        : id_(nid),
          zone_queues_(nid)
    {
        DEBUG_ASSERT(!s_node_list[nid]);
        s_node_list[nid] = this;
    }

    auto PmNode::init(Pfn start, Pfn end) -> Status
    {
        canary_.verify();
        DEBUG_ASSERT(NodeStates::is_offline(id_));

        start_pfn_ = start;
        spanned_frames_ = end - start;
        present_frames_ = 0;

        NodeStates::set_online(id_);
        return Status::Ok;
    }

    auto PmNode::share_with(NodeId nid) -> Status
    {
        DEBUG_ASSERT(id_ != nid);
        DEBUG_ASSERT(NodeStates::is_online(nid));

        if (shared_nodes_.test(nid)) {
            return Status::InvalidArguments;
        }

        auto other_zq = PmNode::node(nid)->zone_queues();

        auto iterator = zone_queues_.iter(ZoneQueues::Local);
        while (auto zoneref = iterator.move_next()) {
            // other_zq->insert(zoneref->zone, zoneref->type);
        }

        shared_nodes_.set(nid);
        return Status::Ok;
    }

    template <typename Inner>
    auto PmNode::alloc_frame_core(ZoneQueues::Iterator<Inner> iter, Gaf flags, usize order) -> ustl::Result<PmFrame *, Status>
    {
        while (auto zoneref = iter.move_next()) {
            auto zone = zoneref->zone;
            if (auto frame = zone->alloc_frame(flags, order)) {
                return ustl::ok(frame);
            }
        }

        return ustl::err(Status::OutOfMem);
    }

    auto PmNode::alloc_frame(Gaf flags, usize order) -> ustl::Result<PmFrame *, Status>
    {
        canary_.verify();

        if (order > MAX_FRAME_ORDER) {
            return ustl::err(Status::InvalidArguments);
        }

        flags &= GAF_ALLOWED;

        auto iterator = zone_queues_.iter(ZoneQueues::NodeAffinity);
        AllocContext context{iterator, order, NodeStates::NODE_STATES[NodeStates::Online]};
        auto frame = this->alloc_frame_core(context.iterator, flags, order);

        return frame;
    }

    auto PmNode::free_frame(PmFrame *frame, usize order) -> void
    {
        auto const ztype = frame->flags().zone_type();
        auto const zone = zone_queues_.local_zone(ztype);

        DEBUG_ASSERT(zone, "");
        zone->free_frame(frame, order);
    }

    auto PmNode::alloc_frames(Gaf flags, usize n, ai_out FrameList<> *list) -> Status
    {
        DEBUG_ASSERT(list != nullptr, "");

        for (usize order = 0; n > 0; order += 1) {
            if ((n & 1) == 0) {
                continue;
            }

            if (auto frame = this->alloc_frame(flags, order)) {
                list->push_back(*frame.unwrap());
                n >>= 1;
            } else {
                this->free_frames(list);
                return frame.unwrap_err();
            }
        }

        return Status::Ok;
    }

    auto PmNode::free_frames(FrameList<> *list) -> void 
    {
        using namespace ustl::function;
        auto const reclaimer = bind(&Self::free_frame, this, _1, 0);
        while (!list->empty()) {
            list->erase_and_dispose(list->begin(), reclaimer);
        }
    }

} // namespace ours::mem