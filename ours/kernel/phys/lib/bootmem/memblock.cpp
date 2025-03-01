#include <bootmem/memblock.hpp>

#include <algorithm>
#include <iterator>

#include <ustl/option.hpp>
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
    auto MemBlock::RegionList::grow(usize new_capacity) -> bool 
    {
        auto base_ = reinterpret_cast<PhysAddr>(regions_);
        auto size_ = capacity_;

        auto const exclude_with_holding = [&] (PhysAddr base, usize size) {
            return base > (base_ + size_ - 1) || base_ > (base + size - 1);
        };

        // Reserved for other ways
        if (auto region = holder_->find_if(exclude_with_holding, new_capacity , PAGE_SIZE, RegionType::Normal)) {
            auto new_regions = reinterpret_cast<Region *>(region);
            ustl::algorithms::copy(begin(), end(), new_regions);

            regions_ = new_regions;
            capacity_ = new_capacity;

            // Give back the memory loaned
            holder_->deallocate(base_, size_);
        }

        UNREACHABLE("");
    }

    FORCE_INLINE
    auto MemBlock::RegionList::insert(usize index, Region const &region) -> void
    {
        total_ += region.size;
    }

    FORCE_INLINE
    auto MemBlock::RegionList::push_back(Region const &region) -> void
    {
        total_ += region.size;
    }

    /// [RegionList]
    auto MemBlock::RegionList::add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status
    {
        if (count_ == 0) {
            push_back({ base, size, type, nid });
            // Insert 
            return Status::Ok;
        }

        auto first = 0, last = 0;
        auto const end = base + size - 1;
        for (usize i = 0; i < count_; ++i) {
            auto const region_base = regions_[i].base;
            auto const region_end = regions_[i].end();
            if (region_base > end) {
                break;
            } else if (region_end <= base) {
                continue;
            }

            // Handle the intersection.
            if (region_base > base) {
                this->insert(i, {base, region_base - base, type, nid });
            }

            base = min(region_base, end);
        }

        if (base < end) {
            push_back({ base, end, type, nid });
        }
        this->try_merge(first, last);

        return Status::Ok;
    }

    auto MemBlock::RegionList::remove(PhysAddr base, usize size) -> Status
    {
        auto [first, last] = this->isolate(base, size);
        if (first == last) {
            return Status::InvalidArguments;
        }
        erase(first, last);
        return Status::Ok;
    }

    auto MemBlock::RegionList::isolate(PhysAddr base, usize size) -> ustl::Pair<IterMut, IterMut>
    {
        if (count_ + 2 < capacity_) {
            this->grow(capacity_ << 1);
        }

        IterMut first, last;
        auto end = base + size - 1;
        for (auto i = 0; i < count_; ++i) {
            auto const region_base = regions_[i].base;
            auto const region_end = regions_[i].end();

            if (region_base > end) {
                break;
            } else if (region_end < base) {
                continue;
            } else if (base < region_base) { // The bottom will to handle the intersection
                regions_[i].base = base;
                regions_[i].size -= region_base - base;
                total_ -= region_base - base;
                this->insert(i, { base, region_base - base, regions_[i].type(), regions_[i].nid() });
            } else if (region_end > end) {
                regions_[i].base = end;
                regions_[i].size -= end - region_base;
                total_ -= end - region_base;
                this->insert(i - 1, {region_base, end - region_base, regions_[i - 1].type(), regions_[i - 1].nid() });
            } else {
                if (last == this->end()) {
                    first = begin() + i;
                }
                last = this->end() + i + 1;
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
            if (next_region->base - curr_region->size != curr_region->base || curr_region->type() != next_region->type()) {
                curr += 1;
                continue;
            }

            curr_region->size += next_region->size;
            erase(next_region);
            last -= 1;
        }

        return Status::Ok;
    }

    auto MemBlock::init(PhysAddr base, usize size) -> void
    {
        auto const half_size = size >> 1;
    }

    auto MemBlock::iterate(IterationContext &context) const -> ustl::Option<Region>
    {
        auto const free_end = memories_.end();
        auto const used_end = reserved_.end();

        for (; context.free_ != free_end; ++context.free_) {
            if (!context.is_fit(*context.free_)) {
                continue;
            }
            if (context.type_ == RegionType::AllType) {
                return { *context.free_ };
            }
            if (context.type_ == RegionType::Unused) {
                auto f_base = context.free_->base, f_end = context.free_->end();
                for (; context.used_!= used_end; ++context.used_) {
                    auto b_base = context.used_->base, b_end = context.used_->end();

                    if (b_base > f_end) {
                        break;
                    } 

                    if (f_base < b_end) { // Got a free area.
                        // Advance one step for next iteration.
                        if (f_end <= b_end) {
                            ++context.free_;
                        } else {
                            ++context.used_;
                        }
                        return {{ 
                            max(f_base, b_base), 
                            min(f_end, b_end), 
                            context.free_->type(), 
                            context.free_->nid() 
                        }};
                    }
                }
            }
        }

        return ustl::NONE;       
    }

    /// [MemBlock]
    auto MemBlock::allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> usize 
    {
        auto const matcher = [start, end] (PhysAddr base_aligned, usize size_aligned) {
            return base_aligned >= start && base_aligned + (size_aligned - 1) < end;
        };
        if (usize candidate = find_if(matcher, size, align, RegionType::Normal)) {
            this->protect(candidate, size);
        }

        return 0;
    }

    auto MemBlock::deallocate(PhysAddr base, usize size) -> void
    {
        this->reserved_.remove(base, size);
    }

} // namespace bootmem