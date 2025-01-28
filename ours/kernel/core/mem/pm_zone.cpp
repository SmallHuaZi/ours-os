#include <ours/mem/pm_zone.hpp>
#include <ours/mem/memory_model.hpp>

#include <ours/early.hpp>
#include <ours/assert.hpp>

#include <algorithm>

#include <ustl/math/log.hpp>

namespace ours::mem {
    PmZone::PmZone(char const *name)
        : name_(name),
          start_pfn_(0),
          present_frames_(0), 
          managed_frames_(0), 
          spanned_frames_(0),
          reserved_frames_(0)
    {}

    // Requires:
    //      1. @map is must zeroed.
    EARLY_CODE 
    auto PmZone::init(NodeId nid, ustl::views::Span<MemRegion> const &ranges) -> void
    {
        DEBUG_ASSERT(ranges.size() != 0, "Flags is unsupported.");

        for (decltype(auto) range: ranges) {
            auto const start_pfn = phys_to_pfn(range.start);
            auto const end_pfn = phys_to_pfn(range.end);
            this->attach_range_locked({ start_pfn, end_pfn });
        }
    }

    // auto PmZone::alloc_frame(usize order) -> PmFrame *
    // {

    // }

    // auto PmZone::free_frame(PmFrame *frame) -> Status 
    // {
    //     return Status::Fail;
    // }

    auto PmZone::attach_range_locked(gktl::Range<Pfn> range) -> Status
    {
        gktl::Range valid_range = range_.right_difference_with(range);
        if (valid_range.is_empty()) {
            return Status::InvalidArguments;
        }

        usize const total_frames = valid_range .length();

        for (usize acc = 0, order = 0; acc < total_frames; acc += 1 << order) {
            usize const tmp = ustl::math::log2(total_frames - acc);
            order = std::min(tmp, Self::MAX_FRAME_ORDER);

            auto pfn = range.start + acc;
            auto frame = MemoryModel::pfn_to_frame(pfn);
            free_list_[order].push_back(*frame);
        }

        present_frames_ += total_frames;

        return Status::Ok;
    }

    auto PmZone::alloc_frame_locked(usize order) -> PmFrame *
    {
        if (order == 0) {
        }

        auto const n = free_list_.size();
        for (auto i = order; i < n; ++i) {
            if (free_list_[i].empty()) {
                continue;
            }
            
            auto frame = &free_list_[i].front();
            free_list_[i].remove(*frame);

            if (i > order) {
                frame = Impl::split(*this, frame, order, i);
            }

            return frame;
        }

        return nullptr;
    }
}