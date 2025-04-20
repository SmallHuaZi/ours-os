#include <ours/mem/pm_zone.hpp>
#include <ours/mem/memory_model.hpp>

#include <ours/init.hpp>
#include <ours/assert.hpp>
#include <ours/cpu-local.hpp>

#include <logz4/log.hpp>

namespace ours::mem {
    FORCE_INLINE
    static auto frame_is_buddy(PmFrame *self, PmFrame *buddy) -> bool {
        if (!buddy->is_role(PfRole::Pmm)) {
            return false;
        }
        if (self->zone() != buddy->zone()) {
            return false;
        }
        if (self->secnum() != buddy->secnum()) {
            return false;
        }
        if (self->nid() != buddy->nid()) {
            return false;
        }
        if (self->order() != buddy->order()) {
            return false;
        }

        return true;
    }

    FORCE_INLINE
    static auto get_buddy(PmFrame *frame, Pfn pfn, usize order) -> PmFrame * {
        auto const buddy_pfn = pfn + (1 << order);
        if (auto buddy_frame = pfn_to_frame(buddy_pfn)) {
            if (frame_is_buddy(frame, buddy_frame)) {
                return buddy_frame;
            }
        }

        return nullptr;
    }

    FORCE_INLINE
    static auto get_buddy_unchecked(PmFrame *frame, Pfn pfn, usize order) -> PmFrame * {
        return &frame[BIT(order)];
    }

    FORCE_INLINE
    auto FrameSet::has_frame(usize order) const -> bool {
        return !lists_[order].empty();
    }

    FORCE_INLINE
    auto FrameSet::get_frame(usize order) -> PmFrame * {
        return &lists_[order].front();
    }

    FORCE_INLINE
    auto FrameSet::remove_frame(PmFrame *frame, usize order) -> void {
        auto const to_erase = lists_[order].iterator_to(*frame);
        lists_[order].erase(to_erase);
        frame->set_role(PfRole::None);
    }

    FORCE_INLINE
    auto FrameSet::insert_frame(PmFrame *frame, usize order) -> void {
        lists_[order].push_back(*frame);
        frame->set_role(PfRole::Pmm);
        frame->set_order(order);
    }

    auto FrameSet::acquire_frame_inner(PmFrame *frame, usize lower_order, usize upper_order) -> PmFrame * {
        auto const pfn = frame_to_pfn(frame);
        while (upper_order > lower_order) {
            upper_order -= 1;

            // In split process, directly taking a buddy frame is absolutely safe.
            auto buddy = get_buddy_unchecked(frame, pfn, upper_order);
            insert_frame(buddy, upper_order);
        }
        frame->set_order(lower_order);

        return frame;
    }

    auto FrameSet::acquire_frame_locked(usize target_order) -> PmFrame * {
        for (auto order = target_order; order < kNumOrders; ++order) {
            if (!has_frame(order)) {
                continue;
            }
            
            auto frame = get_frame(order);
            remove_frame(frame, order);
            acquire_frame_inner(frame, target_order, order);
            return frame;
        }

        return nullptr;
    }


    auto FrameSet::release_frame_inner(PmFrame *frame, Pfn pfn, usize order) -> void {
        // Do fold
        for (auto i = order; i < kMaxFrameOrder; ++i) {
            auto buddy_frame = get_buddy(frame, pfn, order);
            if (!buddy_frame) {
                break;
            }

            remove_frame(buddy_frame, order);
            order += 1;
        }

        insert_frame(frame, order);
    }

    auto FrameSet::release_frame_locked(PmFrame *frame, usize order) -> void {
        auto pfn = frame_to_pfn(frame);
        auto const end_pfn = pfn + (1 << order);

        order = ustl::algorithms::min(order, kMaxFrameOrder);
        while (pfn != end_pfn) {
            release_frame_inner(frame, pfn, order);
            pfn += 1 << order;
            frame = pfn_to_frame(pfn);
        }
    }

    PmZone::PmZone()
        : canary_(),
          name_("Anonymous"),
          type_(ZoneType::MaxNumZoneType),
          nid_(),
          start_pfn_(),
          spanned_frames_(),
          present_frames_(),
          frame_cache_(0)
    {}

    // Requires:
    //      1. @map is must zeroed.
    INIT_CODE 
    auto PmZone::init(NodeId nid, ZoneType type, Pfn start_pfn, Pfn end_pfn, usize present_frames) -> void {
        DEBUG_ASSERT(type < ZoneType::MaxNumZoneType);

        canary_.verify();

        name_ = to_string(type);
        type_ = type;
        nid_ = nid;
        start_pfn_ = start_pfn;
        spanned_frames_ = end_pfn - start_pfn;
        present_frames_ = present_frames;

        frame_cache_ = CpuLocal::allocate<PcpuCache>();
    }

    auto PmZone::finish_allocation(PmFrame *frame, Gaf gaf, usize order) -> void {
        managed_frames_ -= BIT(order);
    }

    auto PmZone::alloc_frame(Gaf gaf, usize order) -> PmFrame * {
        PmFrame *result = nullptr;
        if (is_order_within_pcpu_cache_limit(order)) {
            result = frame_cache_.with_current([order] (PcpuCache &cache) {
                return nullptr;
            });
        }

        if (!result) {
            result = fset_.acquire_frame(order);
        }

        if (result) {
            finish_allocation(result, gaf, order);
        }
        return result;
    }

    auto PmZone::free_frame(PmFrame *frame, usize order) -> void {
        if (is_order_within_pcpu_cache_limit(order)) {
            frame_cache_.with_current([frame] (PcpuCache &cache) {
                // Here no pcpu cache.
            });
        } else {
            fset_.release_frame(frame, order);
        }
        managed_frames_ += BIT(order);
    }
}