#include <bootmem/memblock.hpp>

#include <algorithm>

#include <ustl/option.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/algorithms/search.hpp>
#include <ustl/collections/pmr/memory_resource.hpp>

using std::min;
using std::max;
using ustl::mem::align_up;
using ustl::mem::address_of;

namespace bootmem {
    FORCE_INLINE
    auto MemBlock::RegionList::calc_new_capacity(usize old_capacity) -> usize
    {  return old_capacity << 1;  }

    template <typename F>
    auto MemBlock::RegionList::grow(usize new_capacity, F &&matcher) -> bool
    {
        auto base_ = reinterpret_cast<PhysAddr>(regions_);
        auto size_ = capacity_;
        if (auto new_base = holder_->find_if(matcher, new_capacity , PAGE_SIZE, RegionType::Normal)) {
            reset(reinterpret_cast<Region *>(new_base), new_capacity);
            holder_->protect(new_base, new_capacity);
            // Give back the memory loaned
            holder_->deallocate(base_, size_);

            return true;
        }

        return false;
    }

    FORCE_INLINE
    auto MemBlock::RegionList::grow(usize new_capacity) -> bool 
    {
        auto base_ = reinterpret_cast<PhysAddr>(regions_);
        auto size_ = capacity_;
        auto const exclude_with_holding = [&] (PhysAddr base, usize size) {
            return base > (base_ + size_ - 1) || base_ > (base + size - 1);
        };

        return grow(new_capacity, exclude_with_holding);
    }

    FORCE_INLINE
    auto MemBlock::RegionList::grow(usize new_capacity, PhysAddr reserved_start, usize reserved_size) -> bool 
    {
        auto base_ = reinterpret_cast<PhysAddr>(regions_);
        auto size_ = capacity_;

        auto const matcher = [&] (PhysAddr base, usize size) {
            return base > (base_ + size_ - 1) || base_ > (base + size - 1) &&
                   base > (reserved_start + reserved_size - 1) || reserved_start > (base + size - 1);;
        };

        return grow(new_capacity, matcher);
    }

    FORCE_INLINE
    auto MemBlock::RegionList::reset(Region *new_regions, usize new_capacity) -> void
    {
        ustl::algorithms::copy_n(regions_, count_, new_regions);
        capacity_ = new_capacity;
        regions_ = new_regions;
    }

    FORCE_INLINE
    auto MemBlock::RegionList::insert(usize index, Region const &region) -> void
    {
        if (count_ == capacity_) {
            this->grow(calc_new_capacity(capacity_), region.base, region.size);
        }
        ustl::algorithms::copy_backward(regions_ + index, end(), end() + 1);
        regions_[index] = region;
        total_ += region.size;
        ++count_;
    }

    FORCE_INLINE
    auto MemBlock::RegionList::push_back(Region const &region) -> void
    {
        total_ += region.size;
        if (count_ == capacity_) {
            this->grow(calc_new_capacity(capacity_), region.base, region.size);
        }
        regions_[count_++] = region;
    }

    /// [RegionList]
    auto MemBlock::RegionList::add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status
    {
        // We need to truncate the range
        if (count_ == 0) {
            push_back({ base, size, type, nid });
            // Insert 
            return Status::Ok;
        }

        u32 const sentinel = ustl::NumericLimits<u32>::max();

        PhysAddr const end = base + size;
        u32 first = 0, last = 0, idx = 0;
        for (; idx < count_; ++idx) {
            auto const rbase = regions_[idx].base;
            auto const rend = regions_[idx].end();
            if (rbase > end) {
                // Never overlap with [base, base + size), and be after it.
                break;
            } else if (rend <= base) {
                // Never overlap with [base, base + size), so skip it
                continue;
            }

            // Handle the intersection.
            if (rbase > base) {
                if (first == sentinel) {
                    first = idx;
                }
                last = idx + 1;
                insert(idx, {base, rbase - base, type, nid });
            }

            base = min(rbase, end);
        }

        if (base < end) {
            if (first == sentinel) {
                first = idx;
            }
            last = idx + 1;
            push_back({ base, end - base, type, nid });
        }
        try_merge(first, last);

        return Status::Ok;
    }

    auto MemBlock::RegionList::remove(PhysAddr base, usize size) -> Status
    {
        auto [first, last] = isolate(base, size);
        if (first == last) {
            return Status::InvalidArguments;
        }
        erase(first, last);
        return Status::Ok;
    }

    auto MemBlock::RegionList::isolate(PhysAddr base, usize size) -> ustl::Pair<IterMut, IterMut>
    {
        if (count_ + 2 > capacity_) {
            this->grow(calc_new_capacity(capacity_));
        }

        IterMut first{}, last{};
        auto end = base + size;
        for (auto i = 0; i < count_; ++i) {
            auto const rbase = regions_[i].base;
            auto const rend = regions_[i].end();
            auto const type = regions_[i].type();
            auto const nid = regions_[i].nid();

            if (rbase >= end) {
                break;
            } else if (rend <= base) {
                continue;
            } else if (rbase < base) { // The code bottom will handle the intersection
                // Rectifies this region to [rbase, base)
                auto const left_diff_size = base - rbase;
                regions_[i].base = base;
                regions_[i].size -= left_diff_size;
                total_ -= left_diff_size;
                insert(i, { rbase, left_diff_size, type, nid });
            } else if (rend > end) {
                // Rectifies this region to [end, rend)
                auto const right_diff_size = end - rbase;
                regions_[i].base = end;
                regions_[i].size -= right_diff_size;
                total_ -= right_diff_size;
                insert(i--, {rbase, right_diff_size, type, nid });
            } else {
                // 
                if (!last) {
                    first = begin() + i;
                }
                last = begin() + i + 1;
            }
        }

        return { first, last };
    }

    auto MemBlock::RegionList::try_merge(usize first, usize last) -> Status
    {
        auto curr = 0;
        if (first) {
            curr = first - 1;
        }

        last = min(last, count_ - 1);
        while (curr < last) {
            IterMut curr_region = &regions_[curr];
            IterMut next_region = &regions_[curr + 1];
            if (curr_region->end() != next_region->base || 
                curr_region->type() != next_region->type() ||
                curr_region->nid() != next_region->nid() ) 
            {
                curr += 1;
                continue;
            }

            curr_region->size += next_region->size;
            erase(next_region);
            last -= 1;
        }

        return Status::Ok;
    }

    auto MemBlock::trim(usize align) -> void
    {
        for (auto &region : memories_) {
            auto dup = region;
            region.trim(align);
            if (region.size == 0) {
                this->remove(dup.base, dup.size);
            }
        }
    }

    auto MemBlock::init(Region *new_regions, usize n) -> void
    {
        auto const half_size = n >> 1;
        memories_.reset(new_regions, half_size);
        reserved_.reset(new_regions + half_size, half_size);
    }

    auto MemBlock::build_iteration_context(IterationContext &context) const -> void 
    {
        // Sentiel value for no fit region.
        context.free_ = const_cast<Region *>(memories_.begin());
        context.used_ = const_cast<Region *>(reserved_.begin());
    }

    auto MemBlock::iterate_unused_region(IterationContext &context) const -> ustl::Option<Region>
    {
        auto const mem_end = memories_.end();
        auto const used_end = reserved_.end();

        for (; context.free_ != mem_end; ++context.free_) {
            auto mem_start = context.free_->base;
            auto mem_end = context.free_->end();

            for (; context.used_ != used_end + 1; ++context.used_) {
                // Get the gap between reserved regions.
                PhysAddr gap_start;
                if (context.used_ == reserved_.begin()) {
                    gap_start = start_address_;
                } else {
                    gap_start = (context.used_ - 1)->end();
                }

                PhysAddr gap_end; 
                if (context.used_ - reserved_.end() >= 0) {
                    gap_end = end_address_;
                } else {
                    gap_end = context.used_->base;
                }

                // The bookkepping infomation between the free region and the busy region is
                // in misalignment,  so we need to readjust.
                if (gap_start >= mem_end) {
                    break;
                } 
                // BUG(SmallHuazi) gap_start==gap_end
                // The two regions has intersected with each other. 
                if (mem_start < gap_end) {
                    // Advance one step for next iteration.
                    if (mem_end <= gap_end) {
                        ++context.free_;
                    } else {
                        ++context.used_;
                    }
                    return {{ 
                        max(mem_start, gap_start), 
                        min(mem_end, gap_end), 
                        context.free_->type(), 
                        context.free_->nid() 
                    }};
                }
            }          
        }

        return ustl::NONE;
    }

    auto MemBlock::iterate(IterationContext &context) const -> ustl::Option<Region>
    {
        // For each all unused region is rather complex, so we sperate it to
        // a sub-routine.
        if (context.type_ == RegionType::Unused) {
            return iterate_unused_region(context);
        }

        auto const mem_end = memories_.end();
        while (context.free_ != mem_end) {
            Region *region;
            if (context.type_ == RegionType::Normal) {
                region = context.free_++;
            } else if (context.type_ == RegionType::Reserved) {
                region = context.used_++;
            }

            if (!context.is_range_available(region->base, region->end())) {
                continue;
            }

            // Trim the range
            PhysAddr const start = max(region->base, context.range_.start);
            PhysAddr const end = min(region->end(), context.range_.end);
            return {{ start, end - start, region->type(), region->nid() }};
        }

        return ustl::NONE;
    }

    /// [MemBlock]
    auto MemBlock::allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> usize 
    {
        auto const matcher = [start, end] (PhysAddr base_aligned, usize size_aligned) {
            return base_aligned >= start && base_aligned + size_aligned <= end;
        };
        if (PhysAddr candidate = find_if(matcher, size, align, RegionType::Unused)) {
            if (Status::Ok == protect(candidate, size)) {
                return candidate;
            }
        }

        return 0;
    }

    auto MemBlock::deallocate(PhysAddr base, usize size) -> void
    {
        this->reserved_.remove(base, size);
    }

} // namespace bootmem