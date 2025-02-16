#include <allocator/memblock.hpp>
#include <allocator/eblma.hpp>

#include <algorithm>
#include <iterator>

#include <ustl/option.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/algorithms/search.hpp>
#include <ustl/collections/pmr/memory_resource.hpp>

#include <logz4/log.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(COND, ...)
#endif

#ifndef UNREACHABLE
#   define UNREACHABLE(...) while(1)
#endif

using std::min;
using std::max;
using ustl::mem::align_up;
using ustl::mem::address_of;

namespace allocator {
    /// [BootstrapMemoryResource]
    auto BootstrapMemoryResource::allocate(size_t n, size_t align) -> MemRegion *
    {
        size_t const im_aligned = align_up(size_t(initial_regions_), align);
        size_t const ignore_bytes = im_aligned - size_t(initial_regions_);
        size_t const requested_bytes = sizeof(MemRegion) * n;
        if (requested_bytes < sizeof(initial_regions_) - ignore_bytes) {
            return reinterpret_cast<MemRegion *>(im_aligned);
        }

        auto const ignored_start_ = booking_base_;
        auto const ignored_end_ = booking_size_ + booking_base_;
        auto const matcher = [ignored_start_, ignored_end_ ] (size_t base_aligned, size_t size_aligned, Flags flags) {
            return base_aligned >= ignored_start_ && base_aligned + size_aligned - 1 < ignored_end_;
        };

        constexpr size_t NR_TRIES = 512;
        for (auto i = 0; i < NR_TRIES; ++i) {
            if (auto candidate = memblock_->find_if(matcher, requested_bytes, align)) {
                should_commit_ = true;
                booking_base_ = candidate;
                booking_size_ = requested_bytes;
                return reinterpret_cast<MemRegion *>(candidate);
            }
        }

        UNREACHABLE("");
    }

    inline auto BootstrapMemoryResource::deallocate(MemRegion* p, size_t n) -> void
    {
        size_t const p_us = reinterpret_cast<size_t>(p);
        size_t const im_us = reinterpret_cast<size_t>(initial_regions_);
        size_t const revoked_bytes = sizeof(MemRegion) * n;
        if (p_us > im_us && p_us + revoked_bytes - 1 < im_us + sizeof(initial_regions_) - 1) {
            return;
        }

        memblock_->deallocate(p, revoked_bytes);
    }

    inline auto BootstrapMemoryResource::book(size_t base, size_t size) -> void
    {
        booking_base_ = base;
        booking_size_ = size;
    }

    inline auto BootstrapMemoryResource::commit_booking() -> void
    {
        if (should_commit_) {
            memblock_->protect(booking_base_, booking_size_);
            should_commit_ = false;
            booking_base_ = 0;
            booking_size_ = 0;
        }
    }

    /// [RegionList]
    inline auto RegionList::insert(size_t i, size_t base, size_t size, Flags flags) -> void 
    {
        BootstrapMemoryResource *bootstrap = address_of(inner_.get_stored_allocator());
        bootstrap->book(base, size);
        inner_.insert(begin() + i, { base, size, flags });
        bootstrap->commit_booking();
        total_ += size;
    }

    inline auto RegionList::push_back(size_t base, size_t size, Flags flags) -> void 
    {
        BootstrapMemoryResource *bootstrap = address_of(inner_.get_stored_allocator());
        bootstrap->book(base, size);
        inner_.push_back({base, size, flags});
        bootstrap->commit_booking();
        total_ += size;
    }

    auto RegionList::add(size_t base, size_t size, Flags flags) -> Status
    {
        size_t const n = this->inner_.size();
        if (n == 0) {
            this->push_back(base, size, flags);
            return Status::Ok;
        }

        auto first = 0, last = 0;
        auto const end = base + size - 1;
        for (size_t i = 0; i < inner_.size(); ++i) {
            auto const region_base = inner_[i].base;
            auto const region_end = inner_[i].end();
            if (region_base > end) {
                break;
            } else if (region_end <= base) {
                continue;
            }

            // Handle the intersection.
            if (region_base > base) {
                this->insert(i, base, region_base - base, flags);
            }

            base = min(region_base, end);
        }

        if (base < end) {
            this->push_back(base, end, flags);
        }
        this->try_merge(first, last);

        return Status::Ok;
    }

    auto RegionList::remove(size_t base, size_t size) -> Status
    {
        auto [first, last] = this->isolate(base, size);
        if (first == last) {
            return Status::InvalidRange;
        }
        inner_.erase(first, last);
        return Status::Ok;
    }

    auto RegionList::isolate(size_t base, size_t size) -> ustl::Pair<IterMut, IterMut>
    {
        IterMut first, last;
        auto const n = inner_.size(), end = base + size - 1;
        for (auto i = 0; i < n; ++i) {
            auto const region_base = inner_[i].base;
            auto const region_end = inner_[i].end();

            if (region_base > end) {
                break;
            } else if (region_end < base) {
                continue;
            } else if (base < region_base) { // The bottom will to handle the intersection
                inner_[i].base = base;
                inner_[i].size -= region_base - base;
                total_ -= region_base - base;
                this->insert(i, base, region_base - base, inner_[i].flags);
            } else if (region_end > end) {
                inner_[i].base = end;
                inner_[i].size -= end - region_base;
                total_ -= end - region_base;
                this->insert(i - 1, region_base, end - region_base, inner_[i].flags);
            } else {
                if (last == this->end()) {
                    first = begin() + i;
                }
                last = this->end() + i + 1;
            }
        }

        return { first, last };
    }

    auto RegionList::try_merge(size_t first, size_t last) -> Status
    {
        auto curr = 0;
        if (first) {
            curr = first - 1;
        }

        last = min(last, inner_.size() - 1);
        while (curr < last) {
            auto& curr_region = inner_[curr];
            auto& next_region = inner_[curr + 1];
            if (next_region.base - curr_region.size != curr_region.base ||
                curr_region.flags != next_region.flags)
            {
                curr += 1;
                continue;
            }

            curr_region.size += next_region.size;
            inner_.erase(begin() + curr + 1);
            last -= 1;
        }

        return Status::Ok;
    }

    /// [MemBlock::Cursor]
    auto MemBlock::Cursor::move_next() -> ustl::Option<MemRegion>
    {
        auto const free_end = memblock_->available_list_.end();
        auto const busy_end = memblock_->occupied_list_.end();
        for (; free_iter_ != free_end; ++free_iter_) {
            auto f_base = free_iter_->base, f_end = free_iter_->end();
            for (; busy_iter_ != busy_end; ++busy_iter_) {
                auto b_base = busy_iter_->base, b_end = busy_iter_->end();
                if (b_base > f_end) {
                    break;
                } else if (f_base < b_end) { // Got a free area.
                    // Advance one step for next iteration.
                    if (f_end <= b_end) {
                        ++free_iter_;
                    } else {
                        ++busy_iter_;
                    }
                    return {{ max(f_base, b_base), min(f_end, b_end), free_iter_->flags }};
                }
            }
        }

        return ustl::NONE;
    }

    auto MemBlock::Cursor::move_prev() -> ustl::Option<MemRegion>
    {

    }

    /// [MemBlock]
    auto MemBlock::allocate_bounded(size_t size, size_t align, size_t lowset_limit, size_t uppest_limit) -> size_t 
    {
        Flags out_flags;
        auto const matcher = [lowset_limit, uppest_limit, &out_flags] (size_t base_aligned, size_t size_aligned, Flags flags) {
            out_flags = flags;
            return base_aligned >= lowset_limit && base_aligned + size_aligned - 1 < uppest_limit;
        };
        if (size_t candidate = find_if(matcher, size, align, NO_FLAGS)) {
            this->protect(candidate, size, out_flags);
        }

        return 0;
    }

    auto MemBlock::deallocate(void *ptr, size_t size) -> void
    {
        size_t base = reinterpret_cast<size_t>(ptr);
        this->occupied_list_.remove(base, size);
    }

} // namespace allocator