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

#ifndef OURS_MEM_PM_ZONE_HPP
#define OURS_MEM_PM_ZONE_HPP 1

#include <ours/mem/gaf.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/frame_cache.hpp>

#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/sync/lockguard.hpp>
#include <ustl/collections/array.hpp>
#include <ustl/collections/intrusive/slist.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    /// `PmZone` describes a memory zone with a specific responsibility and 
    /// serves as the core component of the page frame allocator.
    class PmZone
    {
        typedef PmZone     Self;
    public:
        CXX11_CONSTEXPR
        static usize const MAX_PCPU_FRAME_ORDER = 3;

        auto init(NodeId nid, ZoneType ztype, Pfn start_pfn, Pfn end_pfn, usize present_frames = 0) -> void;

        auto contains(usize order) -> bool;

        /// Allocate a frame whose order equals to |order|
        ///
        /// Assumptions:
        ///     1) |order| must be lesser than `MAX_FRAME_ORDER`.
        auto alloc_frame(Gaf gaf, usize order = 0) -> PmFrame *;

        auto free_frame(PmFrame *frame, usize order = 0) -> void;

        auto find_frame(usize order) -> PmFrame *;

        auto which_node() const -> NodeId
        {  return nid_;  }

        auto present_frames() const volatile -> usize
        {  return this->present_frames_;  }

        auto managed_frames() const volatile -> usize
        {  return this->managed_frames_;  }

        auto spanned_frames() const volatile -> usize
        {  return this->spanned_frames_;  }

        auto reserved_frames() const volatile -> usize
        {  return this->reserved_frames_;  }

    private:
        FORCE_INLINE
        static auto is_order_within_pcpu_cache_limit(usize order) -> bool
        {  return order < MAX_PCPU_FRAME_ORDER;  }

        auto init_frame_cache() -> void;

        auto remove_frame_from_free_list(PmFrame *frame, usize order) -> void;

        auto remove_and_split_frame(PmFrame *frame, usize src_order, usize dst_order) -> void;

        auto merge_frame(PmFrame *frame, usize order) -> usize;

        auto expand_frame(PmFrame *frame, usize low_order, usize high_order) -> void;

        auto take_frame_from_free_list_locked(usize order) -> PmFrame *;
        auto take_frame_from_free_list(usize order) -> PmFrame *;

        auto place_frame_to_free_list_locked(PmFrame *frame, usize order) -> void;
        auto place_frame_to_free_list(PmFrame *frame, usize order) -> void;

        auto free_single_frame(PmFrame *frame, Pfn pfn, usize order) -> void;
        auto free_large_frame_block(PmFrame *frame, Pfn pfn, usize order) -> void;
        auto free_frame_inner(PmFrame *frame, Pfn pfn, usize order) -> void;

    protected:
        friend class PmNode;
        GKTL_CANARY(PmZone, canary_);

        char const *name_;

        NodeId  nid_;

        Pfn start_pfn_;

        //! `present_pages` is physical pages existing within the zone, 
        //! which is calculated as:
        //!	    present_frames_ = spanned_frames_ - absent_frames_(pages in holes).
        ustl::sync::AtomicUsize present_frames_;

        //! @brief present pages managed by the buddy system, which
        //! is calculated as (reserved_pages includes pages 
        //! allocated by the booting time allocator):
        //!     managed_frames_ = present_frames_ - reserved_frames_.
        ustl::sync::AtomicUsize managed_frames_;

        ustl::sync::AtomicUsize spanned_frames_;

        ustl::sync::AtomicUsize reserved_frames_;

        ai_percpu FrameCache *frame_cache_;

        ustl::sync::Mutex mutex_;

        ustl::collections::Array<FrameList<>, MAX_FRAME_ORDER> free_list_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_ZONE_HPP