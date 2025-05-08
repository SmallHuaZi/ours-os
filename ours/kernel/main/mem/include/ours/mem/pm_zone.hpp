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
#include <ours/cpu-local.hpp>

#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/sync/lockguard.hpp>
#include <ustl/collections/static-vec.hpp>
#include <ustl/collections/intrusive/slist.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    template <usize MaxOrder>
    class FramePcpuCache {
        ustl::Array<FrameList<>, MaxOrder> lists_;
    };

    class FrameSet {
        typedef FrameSet   Self;
    public:
        CXX11_CONSTEXPR
        static usize const kNumOrders = NR_FRAME_ORDERS;

        CXX11_CONSTEXPR
        static usize const kMaxFrameOrder = MAX_FRAME_ORDER;

        auto acquire_frame(usize order) -> PmFrame * {
            ustl::sync::LockGuard<decltype(mutex_)> guard(mutex_);
            return acquire_frame_locked(order);
        }

        auto release_frame(PmFrame *frame, usize order) -> void {
            ustl::sync::LockGuard<decltype(mutex_)> guard(mutex_);
            return release_frame_locked(frame, order);
        }
    private:
        auto acquire_frame_locked(usize order) -> PmFrame *;

        auto acquire_frame_inner(PmFrame *frame, Pfn pfn, usize order) -> PmFrame *;

        auto release_frame_locked(PmFrame *frame, usize order) -> void;

        auto release_frame_inner(PmFrame *frame, Pfn pfn, usize order) -> void;

        auto remove_frame(PmFrame *frame, usize order) -> void;

        auto insert_frame(PmFrame *frame, usize order) -> void;

        auto has_frame(usize order) const -> bool;

        auto get_frame(usize order) -> PmFrame *;

        ustl::sync::Mutex mutex_;
        ustl::sync::AtomicU8 nr_lists_;
        ustl::Array<FrameList<>, NR_FRAME_ORDERS> lists_;
    };

    // TODO(SmallHuaZi) Seprate the free list from `PmZone` and refactor it to `FrameLists`
    /// `PmZone` describes a memory zone with a specific responsibility and 
    /// serves as the core component of the page frame allocator.
    class PmZone {
        typedef PmZone     Self;
    public:
        /// FIXME(SmallHuaZi) The cache per cpu has not been finished
        CXX11_CONSTEXPR
        static usize const kMaxPcpuCacheOrder = 0;

        enum class WaterMark {
            Critical,   // 0 - %10
            Moderate,   // %10 - %30
            Sufficient, // %30 - %100
            MaxNumMarks,
        };

        PmZone();

        auto init(NodeId nid, ZoneType ztype, Pfn start_pfn, Pfn end_pfn, usize present_frames = 0) -> void;

        auto contains(usize order) -> bool;

        /// Allocate a frame whose order equals to |order|
        ///
        /// Assumptions:
        ///     1) |order| must be lesser than `MAX_FRAME_ORDER`.
        auto alloc_frame(Gaf gaf, usize order = 0) -> PmFrame *;

        auto free_frame(PmFrame *frame, usize order = 0) -> void;

        auto find_frame(usize order) -> PmFrame *;

        FORCE_INLINE CXX11_CONSTEXPR
        auto which_node() const -> NodeId {  
            return nid_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto zone_type() const -> ZoneType {  
            return type_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto present_frames() const volatile -> usize {  
            return this->present_frames_; 
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto managed_frames() const volatile -> usize {  
            return this->managed_frames_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto spanned_frames() const volatile -> usize {  
            return this->spanned_frames_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto reserved_frames() const volatile -> usize {  
            return this->reserved_frames_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto start_pfn() const volatile -> usize {
            return start_pfn_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto end_pfn() const volatile -> usize {
            return start_pfn_ + spanned_frames_;
        }
    private:
        FORCE_INLINE
        static auto is_order_within_pcpu_cache_limit(usize order) -> bool {  
            return order < kMaxPcpuCacheOrder;
        }

        auto finish_allocation(PmFrame *frame, Gaf gaf, usize order) -> void;
    protected:
        friend class PmNode;
        GKTL_CANARY(PmZone, canary_);

        char const *name_;
        NodeId  nid_;
        ZoneType type_;
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

        ustl::Array<usize, usize(WaterMark::MaxNumMarks)> watermark_;

        typedef FramePcpuCache<kMaxPcpuCacheOrder>  PcpuCache;
        PerCpu<PcpuCache> frame_cache_;

        FrameSet fset_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_ZONE_HPP