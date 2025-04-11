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
#include <ours/mem/frame_queue.hpp>

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
#include <ustl/collections/intrusive/slist.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/util/pair.hpp>

#include <gktl/canary.hpp>

#include <kmrd/damon.hpp>

namespace ours::mem {
    struct NodeStates {
        enum NodeStateType {
            DmaMemory = ZoneType::Dma,

            Dma32Memory = ZoneType::Dma32,

            NormalMemory = ZoneType::Normal,

            // When a node created but not initialized, it's state is possible,
            // Meaning that it become a online probablely during system runtime.
            Possible,

            // When a node created and initialized, it's state is online.
            // Meaning that it is active in system, so the de/allocation requests
            // on it is feasible.
            Online,
            MaxNumStateType,
        };

        FORCE_INLINE
        static auto is_state(NodeId nid, NodeStateType state) -> bool {
            return NODE_STATES[state].test(nid);
        }

        FORCE_INLINE
        static auto is_online(NodeId nid) -> bool {
            return is_state(nid, Online);
        }

        FORCE_INLINE
        static auto is_offline(NodeId nid) -> bool
        {  return !is_online(nid);  }

        FORCE_INLINE
        static auto is_possible(NodeId nid) -> bool
        {  return is_state(nid, Possible);  }

        FORCE_INLINE
        static auto set_state(NodeId nid, NodeStateType state) -> void
        {  NODE_STATES[state].set(nid);  }

        FORCE_INLINE
        static auto set_online(NodeId nid) -> void
        {  set_state(nid, Online);  }

        FORCE_INLINE
        static auto set_offline(NodeId nid) -> void
        {  NODE_STATES[Online].set(nid, 0);  }

        FORCE_INLINE
        static auto set_possible(NodeId nid) -> void
        {  set_state(nid, Possible);  }

        template <typename F>
            requires ustl::traits::Invocable<F, NodeId>
        FORCE_INLINE
        static auto for_each_online(F &&functor)
        {  NODE_STATES[Online].for_each(functor);  }

        template <typename F>
            requires ustl::traits::Invocable<F, NodeId>
        FORCE_INLINE
        static auto for_each_possible(F &&functor)
        {  NODE_STATES[Possible].for_each(functor);  }

        using Inner = ustl::collections::StaticVec<NodeMask, MaxNumStateType>;
        static Inner NODE_STATES;
    };

    /// `ZoneQueues` is a helper class separated from `PmNode`, specifically
    /// designed to manage zones of different priorities within a node and provide
    /// cross-node zone access and communication capabilities.
    struct ZoneQueues {
        struct ZoneRef {
            PmZone *zone;
            ZoneType type;
        };
        typedef ustl::collections::StaticVec<ZoneRef, MAX_ZONES>           GlobalQueue;
        typedef ustl::collections::StaticVec<ZoneRef, NR_ZONES_PER_NODE>   LocalQueue;

        static_assert(ustl::traits::IsSameV<GlobalQueue::IterMut, LocalQueue::IterMut>);

        /// `Iterator`
        template <typename Inner>
        struct Iterator;
        typedef Iterator<GlobalQueue::IterMut>      IterMut;
        typedef Iterator<GlobalQueue::RevIterMut>   RevIterMut;

        enum QueueType {
            NodeAffinity,
            ZonePriority,
            MaxNumGlobalQueue,
            Local = MaxNumGlobalQueue,
        };

        ZoneQueues(NodeId nid);

        auto emplace(PmZone *zone, ZoneType ztype) -> void;

        auto insert_queue(LocalQueue const &queue) -> void;

        auto remove(PmZone *zone) -> void;

        auto local_zone(ZoneType ztype) -> PmZone *
        {  return local_zones_[ztype]; }

        auto iter(QueueType ztype) -> IterMut;

        auto rev_iter(QueueType type) -> RevIterMut;

    private:
        template <QueueType Type>
        auto priv_insert_queue(LocalQueue const &queue) -> void;

        template <QueueType Type>
        auto priv_remove(PmZone *zone) -> void;

        NodeId nid_;
        ustl::collections::StaticVec<PmZone *, NR_ZONES_PER_NODE>   local_zones_;
        ustl::collections::StaticVec<ZoneRef, NR_ZONES_PER_NODE>    local_queue_;
        ustl::collections::StaticVec<ZoneRef, MAX_ZONES>            node_affinity_queue_;
        ustl::collections::StaticVec<PmZone *, NR_ZONES_PER_NODE, MAX_NODES> zone_priority_queue_;
    };

    /// `PmNode` is a class that describes a NUMA domain.
    ///
    /// The memory regions between nodes should be mutually exclusive,
    /// meaning that the memory region of one node should not overlap
    /// with the memory region of another node.
    class PmNode {
        typedef PmNode     Self;
    public:
        FORCE_INLINE
        static auto node(NodeId nid) -> PmNode *
        {  return s_node_list[nid];  }

        FORCE_INLINE
        static auto distance(NodeId x, NodeId y) -> usize {
            DEBUG_ASSERT(x < MAX_NODES && y < MAX_NODES, "");
            return s_node_distance[x][y];
        }

        FORCE_INLINE
        static auto distance(Self const &x, Self const &y) -> usize {
            return Self::distance(x.nid(), y.nid());
        }

        FORCE_INLINE
        static auto set_distance(NodeId x, NodeId y, usize dis) -> void {
            DEBUG_ASSERT(x < MAX_NODES && y < MAX_NODES, "");
            s_node_distance[x][y] = dis;
        }

        /// Initialize this node. The primary task involves verifying the existence of the
        /// specified PFN range.
        ///
        /// Requires:
        ///     1). Range [start, end) is sorted by address.
        auto init(Pfn start, Pfn end) -> Status;

        /// This allows this node to share with another node whose id is equal to |nid| its local memory.
        ///
        auto share_with(NodeId nid) -> Status;

        auto alloc_frame(Gaf flags, usize order = 0) -> ustl::Result<PmFrame *, Status>;

        auto free_frame(PmFrame *frame, usize order) -> void;

        auto alloc_frames(Gaf flags, usize n, ai_out FrameList<> *out) -> Status;

        auto free_frames(FrameList<> *list) -> void;

        auto contains(usize order, ZoneType type) -> bool;

        FORCE_INLINE
        auto nid() const -> NodeId
        {  return this->id_;  }

        FORCE_INLINE
        auto zone_queues() -> ZoneQueues *
        {  return &zone_queues_;  }

        auto recalculate_present_frames();

        PmNode(NodeId nid);
    private:
        /// This method is marked `template` to avoid giving the concrete type in the header.
        /// We put the implementation on source files
        template <typename Inner>
        auto alloc_frame_core(ZoneQueues::Iterator<Inner>, Gaf, usize) -> ustl::Result<PmFrame *, Status>;

        GKTL_CANARY(PmNode, canary_);

        NodeId id_;

        Pfn start_pfn_;

        ustl::sync::AtomicUsize spanned_frames_;

        //! @brief present_pages is physical pages existing within the zone,
        //! which is calculated as:
        //!	    present_frames_ = spanned_frames_ - absent_frames_(pages in holes).
        ustl::sync::AtomicUsize present_frames_;

        ustl::sync::AtomicUsize reserved_frames_;

        NodeMask shared_nodes_;

        ZoneQueues zone_queues_;

        FrameQueue lru_queue_;

        using NodeList = ustl::Array<PmNode *, MAX_NODES>;
        static inline NodeList s_node_list;

        // Th fields bottom is readonly after initializing logically.
        using DisMap = ustl::Array<usize, MAX_NODES, MAX_NODES>;
        static inline DisMap s_node_distance;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_NODE_HPP