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

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/node_mask.hpp>
#include <ours/mem/frame_queue.hpp>
#include <ours/mem/memory_priority.hpp>

#include <ours/assert.hpp>
#include <ours/cpu.hpp>
#include <ours/early.hpp>
#include <ours/marco_abi.hpp>

#include <ustl/views/span.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/collections/array.hpp>
#include <ustl/traits/is_invocable.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    class ZoneListPerNode
    {
        typedef ZoneListPerNode   Self;
    public:
        typedef ustl::collections::Array<PmZone *, MAX_NR_ZONES>            ZoneList;
        typedef ustl::collections::Array<PmZone, MAX_NR_ZONES_PER_NODE>     LocalZoneSet;

        auto pick_zone(Gaf flags, usize order) -> PmZone *;
    
        auto pick_zone_list_by_gaf(Gaf flags) -> ZoneList *;

    private:
        enum ZoneListType { LocalZoneList, GlobalZoneList, ZoneListCount };
        ZoneList zone_list_[ZoneListCount];

        LocalZoneSet local_zones_;
    };

    inline auto ZoneListPerNode::pick_zone_list_by_gaf(Gaf flags) -> ZoneList *
    {
        auto index = ZoneListType::GlobalZoneList;
        if (bool(flags & Gaf::OnlyThisNode)) {
            index = ZoneListType::LocalZoneList;
        }

        return &zone_list_[index];
    }

    /// `PmNode` is a class that describes a NUMA domain.
    ///
    /// The memory regions between nodes should be mutually exclusive, 
    /// meaning that the memory region of one node should not overlap 
    /// with the memory region of another node.
    class PmNode
    {
        typedef PmNode     Self;
    public:
        FORCE_INLINE
        static auto node(NodeId nid) -> PmNode *
        {  return Self::ALL_NODE_LIST[nid.inner];  }

        FORCE_INLINE
        static auto distance(Self const &x, Self const &y) -> isize
        {  return Self::NODE_DISTANCE[x.id_.inner][y.id_.inner];  }

        /// Check if the given address `addr` is been managing by physcal memory manager(PmNode)
        static auto is_address_managed(PhysAddr addr) -> bool;

        template <typename Functor> 
            requires ustl::traits::IsInvocableV<Functor, Self *>
        static auto for_each_all_online_nodes(Functor functor) -> void;

        template <typename Functor> 
            requires ustl::traits::IsInvocableV<Functor, Self *>
        static auto for_each_all_possible_node(Functor functor) -> void;

        /// Initialize the node by 
        ///
        /// Requires:
        ///     1). `ranges` is sorted by address.
        auto init(NodeId id) -> Status;

        /// Attach a memory range to the node.
        ///
        /// This method is used in the HOTPLUG memory management to dynamically
        /// add memory to a node.
        auto attach_range(PhysAddr base, usize len, MemoryPriority = MemoryPriority::Max) -> Status;

        auto attach_range_unchecked(PhysAddr base, usize len, MemoryPriority = MemoryPriority::Max) -> Status;

        /// Detach a memory range from the node.
        ///
        /// This method is used in the HOTPLUG memory management to dynamically
        /// remove memory from a node.
        auto detach_range(PhysAddr base, usize len) -> Status;

        auto alloc_frame(Gaf flags, usize order = 0) -> ustl::Result<PmFrame *>;

        auto free_frame(PmFrame *) -> Status;

        template <typename... Options>
        auto alloc_frames(Gaf flags, usize n, ai_out FrameList<Options...> *out) -> Status;

        template <typename... Options>
        auto free_frames(FrameList<Options...> *list) -> Status;

        auto contains(Gaf flags, usize order) -> bool;

        auto nid() -> NodeId
        {  return this->id_;  }

        auto gaf() -> Gaf
        {  return this->allowed_gaf_;  }

    private:
        auto find_best_zone(Gaf flags, usize order) -> PmZone *;

        static auto rebuild_zone_list(Self *node) -> void;

    private:
        GKTL_CANARY(PmNode, canary_);

        /// (NodeId) It points out which cpu the node is going on.
        NodeId id_;

        Pfn start_pfn_;

        ustl::sync::AtomicUsize spanned_frames_;

        //! @brief present_pages is physical pages existing within the zone,
        //! which is calculated as:
        //!	    present_frames_ = spanned_frames_ - absent_frames_(pages in holes).
        ustl::sync::AtomicUsize present_frames_;

        ustl::sync::AtomicUsize reserved_frames_;

        Gaf allowed_gaf_;

        FrameQueue lru_queue_;

        ZoneListPerNode zone_set_;
        ustl::collections::Array<PmZone, MAX_NR_ZONES_PER_NODE> native_zones_;

        static NodeMask ONLINE_NODE_MASK;
        static ustl::collections::Array<PmNode *, MAX_NR_NODES> ALL_NODE_LIST;

        // Th fields bottom is readonly after initializing logically.
        static ustl::collections::Array<ustl::collections::Array<usize, MAX_NR_NODES>, MAX_NR_NODES> NODE_DISTANCE;
    };

    template <typename Functor>
        requires ustl::traits::IsInvocableV<Functor, PmNode *>
    auto PmNode::for_each_all_online_nodes(Functor functor) -> void
    {
        auto const n = ONLINE_NODE_MASK.size();
        for (auto i = 0; i < n; ++i) {
            if (ONLINE_NODE_MASK[i]) {
                ustl::function::invoke(functor, ALL_NODE_LIST[i]);
            }
        }
    }

    template <typename Functor> 
        requires ustl::traits::IsInvocableV<Functor, PmNode *>
    auto PmNode::for_each_all_possible_node(Functor functor) -> void
    {}

    template <typename... Options>
    auto PmNode::alloc_frames(Gaf flags, usize n, ai_out FrameList<Options...> *list) -> Status
    {
        DEBUG_ASSERT(list != nullptr, "");
        for (usize order = 0; n > 0; order += 1) {
            if ((n & 1) == 0) {
                continue;
            }

            ustl::Result<PmFrame *> frame = this->alloc_frame(flags, order);
            if (frame) {
                list->push_back(frame.unwrap());
                n >>= 1;
            } else {
                this->free_frames(list);
                return frame.error();
            }
        }

        return Status::Ok;
    }

    template <typename... Options>
    auto PmNode::free_frames(FrameList<Options...> *list) -> Status
    {
        for (auto first = list->begin(), last = list->end(); first != last; ++first) {
            auto frame = std::addressof(*first);
            free_frame(frame);
        }
        
        return Status::Ok;
    }

    inline auto PmNode::is_address_managed(PhysAddr addr) -> bool
    {
        bool ans = false;
        PmNode::for_each_all_online_nodes([addr, &ans] (PmNode *node) {
            auto pfn = phys_to_pfn(addr);
            ans |= node->start_pfn_ <= pfn && pfn < (node->start_pfn_ + node->spanned_frames_);
        });

        return ans;
    }

    inline auto PmNode::attach_range(PhysAddr base, usize len, MemoryPriority priority) -> Status
    {
        DEBUG_ASSERT(usize(priority) <= usize(MemoryPriority::Max));

        if (Self::is_address_managed(base) || Self::is_address_managed(base + len - 1)) {
            return Status::Fail;
        }
        return this->attach_range_unchecked(base, len, priority);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_NODE_HPP