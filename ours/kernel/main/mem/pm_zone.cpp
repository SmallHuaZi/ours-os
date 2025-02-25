#include <ours/mem/pm_zone.hpp>
#include <ours/mem/memory_model.hpp>

#include <ours/init.hpp>
#include <ours/assert.hpp>

namespace ours::mem {
    FORCE_INLINE
    static auto frame_is_buddy(PmFrame *self, PmFrame *buddy) -> bool 
    {
        if (!buddy->is_role(PfRole::Pmm)) {
            return false;
        }
        if (self->zone_type() != buddy->zone_type()) {
            return false;
        }

        return true;
    }

    FORCE_INLINE
    static auto get_buddy(PmFrame *frame, Pfn pfn, usize order) -> PmFrame *
    {
        auto buddy_pfn = pfn + (1 << (order - 1));
        if (auto buddy_frame = MemoryModel::pfn_to_frame(buddy_pfn)) {
            if (frame_is_buddy(frame, buddy_frame)) {
                return buddy_frame;
            }
        }

        return 0;
    }

    INIT_CODE 
    auto PmZone::init_frame_cache() -> void
    {}

    // Requires:
    //      1. @map is must zeroed.
    INIT_CODE 
    auto PmZone::init(NodeId nid, ZoneType type, Pfn start_pfn, Pfn end_pfn, usize present_frames) -> void
    {
        DEBUG_ASSERT(type < ZoneType::MaxNumZoneType);

        canary_.verify();

        name_ = to_string(type);
        nid_ = nid;
        start_pfn_ = start_pfn;
        spanned_frames_ = end_pfn - start_pfn;
        present_frames_ = present_frames;

        init_frame_cache();
    }

    /// |frame| do not exists in `free_list_`
    ///
    ///
    FORCE_INLINE
    auto PmZone::remove_frame_from_free_list(PmFrame *frame, usize order) -> void
    {
        auto const to_erase = free_list_[order].iterator_to(*frame);
        free_list_[order].erase(to_erase);
    }

    auto PmZone::expand_frame(PmFrame *frame, usize low_order, usize high_order) -> void
    {
        DEBUG_ASSERT(low_order >= 0);
        DEBUG_ASSERT(high_order > low_order);

        auto const pfn = MemoryModel::frame_to_pfn(frame);
        for (auto order = high_order - 1; order >= low_order; --order) {
            if (auto buddy_frame = get_buddy(frame, pfn, order)) {
                free_list_[order].push_back(*frame);
            }
        }
    }

    FORCE_INLINE
    auto PmZone::remove_and_split_frame(PmFrame *frame, usize order, usize target_order) -> void
    {
        remove_frame_from_free_list(frame, order);
        return expand_frame(frame, order, target_order);
    }

    auto PmZone::take_frame_from_free_list_locked(usize order) -> PmFrame *
    {
        auto const max_order  = free_list_.size();
        for (auto i = order; i < max_order; ++i) {
            if (free_list_[i].empty()) {
                continue;
            }
            
            auto frame = &free_list_[i].front();
            remove_and_split_frame(frame, i, order);
            return frame;
        }

        return nullptr;
    }

    FORCE_INLINE
    auto PmZone::take_frame_from_free_list(usize order) -> PmFrame *
    {
        ustl::sync::LockGuard<decltype(mutex_)> guard(mutex_);
        return this->take_frame_from_free_list_locked(order);
    }

    auto PmZone::alloc_frame(Gaf gaf, usize order) -> PmFrame *
    {
        if (is_order_within_pcpu_cache_limit(order)) {
            if (auto frame = frame_cache_[order].take_object()) {
                return frame;
            }
        }

        return this->take_frame_from_free_list(order);
    }

    auto PmZone::free_single_frame(PmFrame *frame, Pfn pfn, usize order) -> void
    {
        auto const nr_orders  = free_list_.capacity();

        // Do fold
        for (auto i = order; i < nr_orders; ++i) {
            auto buddy_frame = get_buddy(frame, pfn, order);
            if (!buddy_frame) {
                break;
            }

            remove_frame_from_free_list(buddy_frame, order);
            order += 1;
        }

        free_list_[order].push_back(*frame);
    }

    auto PmZone::free_large_frame_block(PmFrame *frame, Pfn pfn, usize order) -> void
    {
        auto const end_pfn = pfn + (1 << order);
        auto const max_order = free_list_.capacity() - 1;

        order = ustl::algorithms::min(order, max_order);

        while (pfn != end_pfn) {
            free_single_frame(frame, pfn, order);
            pfn += 1 << order;
            frame = MemoryModel::pfn_to_frame(pfn);
        }
    }

    FORCE_INLINE
    auto PmZone::free_frame_inner(PmFrame *frame, Pfn pfn, usize order) -> void 
    {
        if (is_order_within_pcpu_cache_limit(order)) {
            return frame_cache_[order].return_object(frame);
        }

        ustl::sync::LockGuard<decltype(mutex_)> guard(mutex_);
        free_large_frame_block(frame, pfn, order);
    }

    auto PmZone::free_frame(PmFrame *frame, usize order) -> void
    {
        auto const pfn = MemoryModel::frame_to_pfn(frame);
        free_frame_inner(frame, pfn, order);

        auto const nr_pages = 1 << order;
        this->managed_frames_ += nr_pages;
    }

}