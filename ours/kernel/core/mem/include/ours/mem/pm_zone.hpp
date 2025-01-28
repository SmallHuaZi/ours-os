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

#include <ours/status.hpp>

#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/sync/lockguard.hpp>
#include <ustl/collections/array.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    class alignas(16) PmZone
    {
        typedef PmZone     Self;
    public:
        enum Types: usize;

        CXX11_CONSTEXPR
        static usize const MAX_FRAME_ORDER = 11;

        PmZone(char const *name);

        auto init(NodeId nid, ustl::views::Span<MemRegion> const &ranges) -> void;

        auto contains(usize order) -> bool;

        auto alloc_frame(usize order = 0) -> PmFrame *;

        auto free_frame(PmFrame *frame) -> Status;

        auto attach_range(gktl::Range<Pfn> range) -> Status;

        auto detach_range(gktl::Range<Pfn> range) -> Status;

        auto find_frame(usize order) -> PmFrame *;

        auto present_frames() const -> usize
        {  return this->present_frames_;  }

        auto managed_frames() const -> usize
        {  return this->managed_frames_;  }

        auto spanned_frames() const -> usize
        {  return this->spanned_frames_;  }

        auto reserved_frames() const -> usize
        {  return this->reserved_frames_;  }

    private:
        auto alloc_frame_locked(usize order) -> PmFrame *;

        auto alloc_frame_inner(usize order) -> PmFrame *;

        auto free_frame_locked(PmFrame *) -> Status;

        auto release_frame_locked(PmFrame *, usize order) -> void;

        ai_unsafe auto attach_range_unchecked(gktl::Range<Pfn> range) -> Status;

        ai_unsafe auto detach_range_unchecked(gktl::Range<Pfn> range) -> Status;
    
        ai_unsafe auto attach_range_locked(gktl::Range<Pfn> range) -> Status;

        ai_unsafe auto detach_range_locked(gktl::Range<Pfn> range) -> Status;

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

        ustl::sync::Mutex mutex_;

        FrameCache minimum_frame_cache_;

        ustl::collections::Array<FrameList<>, MAX_FRAME_ORDER> free_list_;
    };

    inline auto PmZone::attach_range(gktl::Range<Pfn> range) -> Status
    {
        ustl::sync::LockGuard<decltype(mutex_)> lock(mutex_);
        return this->attach_range_locked(range);
    }

    inline auto PmZone::detach_range(gktl::Range<Pfn> range) -> Status
    {
        ustl::sync::LockGuard<decltype(mutex_)> lock(mutex_);
        return this->detach_range_locked(range);
    }

    inline auto PmZone::alloc_frame(usize order) -> PmFrame *
    {
        ustl::sync::LockGuard<decltype(mutex_)> lock(mutex_);  
        return this->alloc_frame_locked(order);
    }

    inline auto PmZone::free_frame(PmFrame *frame) -> Status
    {
        ustl::sync::LockGuard<decltype(mutex_)> lock(mutex_);  
        return this->free_frame_locked(frame);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_ZONE_HPP