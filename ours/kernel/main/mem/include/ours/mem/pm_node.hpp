/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef OURS_MEM_PM_NODE_HPP
#define OURS_MEM_PM_NODE_HPP 1

#include <ours/mem/gaf.hpp>
#include <ours/mem/node-mask.hpp>
#include <ours/mem/node-states.hpp>
#include <ours/mem/page_queues.hpp>

#include <ours/assert.hpp>
#include <ours/cpu.hpp>
#include <ours/init.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/array.hpp>
#include <ustl/result.hpp>
#include <ustl/views/span.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/function/bind.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/collections/vec.hpp>
#include <ustl/collections/static-vec.hpp>
#include <ustl/collections/intrusive/slist.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/util/pair.hpp>
#include <ustl/iterator/traits.hpp>

#include <gktl/canary.hpp>

#include <kmrd/damon.hpp>

namespace ours::mem {
    struct NodeRoute {
        NodeRoute()
            : count_(),
              path_()
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_next(NodeId to) -> void {
            count_ += 1;
            path_.push_back(to);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto clear() -> void {
            path_.clear();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto begin() {
            return path_.begin();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto end() {
            return begin() + count_;
        }

        usize count_;
        ustl::collections::StaticVec<usize, MAX_NODE - 1> path_;
    };

    /// `ZoneQueues` is a helper class separated from `PmNode`, specifically
    /// designed to manage zones of different priorities within a node and provide
    /// cross-node zone access and communication capabilities.
    struct ZoneQueues {
        typedef PmZone *    ZoneRef;
        typedef ustl::views::Span<ZoneRef>  ZoneQueue;

        enum QueueType {
            /// A zone queue which arranged by zone type, there may be holes if a type of zone do not exist
            /// in the node. It's just useful when we free a frame.
            LocalSequential,
            LocalContiguous,
            SharedAffinity,
            SharedSequential,
            MaxNumQueues,
        };

        ZoneQueues(NodeId nid) 
            : nid_(nid),
              queues_(),
              queue_storage_(),
              connected_(),
              queue_sizes_()
        { init_queues(); } 

        FORCE_INLINE CXX11_CONSTEXPR
        auto insert_local_zone(PmZone *zone, ZoneType ztype) -> void {
            DEBUG_ASSERT(ZoneTypeVal(ztype) < NR_ZONES_PER_NODE);
            DEBUG_ASSERT(get_queue(QueueType::LocalSequential)[ZoneTypeVal(ztype)] == 0);
            DEBUG_ASSERT(zone != 0);

            get_queue(QueueType::LocalSequential)[ZoneTypeVal(ztype)] = zone;
            push_back(zone, QueueType::LocalContiguous);
            push_back(zone, QueueType::SharedAffinity);
            set_node_state(nid_, NodeStates::Type(ztype), true);
        }

        /// Call it after init_queues
        auto connect(NodeMask const &nodemask) -> void;

        auto dump_queue(QueueType type) const -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_local_zone(ZoneType ztype) -> ZoneRef {
            DEBUG_ASSERT(ZoneTypeVal(ztype) < NR_ZONES_PER_NODE);
            return get_queue(QueueType::LocalSequential)[ZoneTypeVal(ztype)];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_queue(QueueType type) -> ZoneQueue {
            return queues_[type].subspan(0, get_queue_size(type));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_queue(QueueType type) const -> ZoneQueue {
            return queues_[type].subspan(0, get_queue_size(type));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_queue_size(QueueType type) const -> usize {
            return queue_sizes_[type];
        }
    private:
        auto init_queues() -> void;

        auto insert_remote_queue(ZoneQueue const &queue) -> void;

        template <QueueType Type>
        auto insert_remote_queue(ZoneQueue const &remote_queue) -> void;

        template <QueueType Type>
        auto priv_remove_zone(PmZone *zone) -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        auto push_back(PmZone *zone, QueueType type) -> void {
            queues_[type][queue_sizes_[type]] = zone;
            queue_sizes_[type] += 1;
        }

        CXX11_CONSTEXPR
        static usize const kNumLocalQueues = SharedAffinity;

        CXX11_CONSTEXPR
        static usize const kMaxZoneRef = {
            MAX_NODE * (MaxNumQueues - kNumLocalQueues) + (NR_ZONES_PER_NODE * kNumLocalQueues)
        };

        ustl::Array<PmZone *, kMaxZoneRef> queue_storage_;
        usize queue_sizes_[MaxNumQueues];
        ZoneQueue queues_[MaxNumQueues];
        NodeMask connected_;
        NodeId nid_;
    };

    struct AllocationContext;

    /// `PmNode` is a class that describes a NUMA domain.
    ///
    /// The memory regions between nodes should be mutually exclusive,
    /// meaning that the memory region of one node should not overlap
    /// with the memory region of another node.
    class PmNode {
        typedef PmNode     Self;
    public:
        PmNode(NodeId nid);

        /// Initialize this node. The primary task involves verifying the existence of the
        /// specified PFN range.
        ///
        /// Requires:
        ///     1). Range [start, end) is sorted by address.
        auto init(Pfn start, Pfn end) -> Status;

        auto alloc_frame(Gaf gaf, usize order = 0) -> ustl::Result<PmFrame *, Status> {
            return alloc_frame(gaf, order, node_online_mask()) ;
        }

        auto alloc_frame(Gaf flags, usize n, NodeMask const &mask) -> ustl::Result<PmFrame *, Status>;

        /// Allocate `|n|` of frames and do best to
        auto alloc_frames(Gaf flags, usize n, ai_out FrameList<> *out, NodeMask const &mask) -> Status;

        /// Allocate `|n|` of frames and do best to
        auto alloc_frames_bulk(Gaf flags, usize n, ai_out FrameList<> *out) -> Status;

        /// Free a frame, 
        static auto free_frame(PmFrame *frame, usize order) -> void;

        /// Free a list of frame, 
        static auto free_frames(FrameList<> *list) -> void;

        auto contains(usize order, ZoneType type) -> bool;

        auto dump() const -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        auto nid() const -> NodeId {
            return this->id_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto native_cpus() -> CpuMask & {
            return native_cpus_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto native_cpus() const -> CpuMask const & {
            return native_cpus_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto zone_queues() -> ZoneQueues * {
            return &zone_queues_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto node(NodeId nid) -> PmNode * {
            return s_node_list[nid];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto num_nodes() -> usize {
            return s_num_nodes;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto distance(NodeId x, NodeId y) -> usize {
            DEBUG_ASSERT(x < MAX_NODE && y < MAX_NODE, "");
            return s_node_distance[x][y];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto distance(Self const &x, Self const &y) -> usize {
            return Self::distance(x.nid(), y.nid());
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto set_distance(NodeId x, NodeId y, usize dis) -> void {
            DEBUG_ASSERT(x < MAX_NODE && y < MAX_NODE, "");
            s_node_distance[x][y] = dis;
        }

        static auto build_optimal_node_route(NodeId from, NodeRoute &path, NodeMask const &filter) -> void;
    private:
        friend class ZoneQueues;

        auto alloc_frame_core(Gaf gaf, usize order, AllocationContext &context) -> PmFrame *;

        auto finish_allocation(PmFrame *frame, Gaf gaf, usize order, AllocationContext const &context) -> void;

        GKTL_CANARY(PmNode, canary_);

        NodeId id_;

        CpuMask native_cpus_;

        Pfn start_pfn_;

        ustl::sync::AtomicUsize spanned_frames_;

        //! @brief present_pages is physical pages existing within the zone,
        //! which is calculated as:
        //!	    present_frames_ = spanned_frames_ - absent_frames_(pages in holes).
        ustl::sync::AtomicUsize present_frames_;

        ustl::sync::AtomicUsize reserved_frames_;

        ZoneQueues zone_queues_;
        PageQueues page_queues_;

        using NodeList = ustl::Array<PmNode *, MAX_NODE>;
        static inline NodeList s_node_list;

        // Th fields bottom is readonly after initializing logically.
        using DisMap = ustl::Array<usize, MAX_NODE, MAX_NODE>;
        static inline DisMap s_node_distance;

        // Th fields bottom is readonly after initializing logically.
        static inline usize s_num_nodes;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_NODE_HPP