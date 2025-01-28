// There are six following kinds of case:
//  1. [nb ... [sb ... se] ... ne] --embedded;
//  2. [sb ... [nb ... ne] ... se] --embedded;
//  3. [nb ... [sb ... ne] ... se] --overlapped;
//  4. [sb ... [nb ... se] ... ne] --overlapped;
//  5. [nb ... ne] [sb ... se] --independent;
//  6. [sb ... se] [nb ... ne] --independent.
// Bugs:
//  1. We suppose that `list` is able to store all sections to add, 
//     but actually it maybe has no so much of memory space.
//  2. `new_section.base_` and `.end_` maybe is unaligned by `PAGE_SIZE`.

#include <allocator/memblock.hpp>

#include <algorithm>
#include <iterator>

#include <ustl/option.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/algorithms/search.hpp>
#include <ustl/collections/pmr/monotonic_buffer_resource.hpp>

#include <logz4/log.hpp>

using std::min;
using std::max;
using std::clamp;
using std::find_if;
using std::for_each;
using std::distance;
using ustl::mem::align_down;
using ustl::mem::align_up;
using ustl::mem::address_of;
using ustl::mem::construct_at;
using ustl::algorithms::find_if;
using ustl::collections::pmr::MonotonicBufferResource;

namespace allocator {
    constexpr static size_t DEFAULT_CACHE_SIZE = 512; 
    static MemRegion REGION_CACHE[DEFAULT_CACHE_SIZE];
    static MonotonicBufferResource DEFAULT_MBR;

    MemRegionList::MemRegionList()
    {
        construct_at(&DEFAULT_MBR, REGION_CACHE, sizeof(REGION_CACHE));
    }

    auto MemRegionList::add(size_t base, size_t size, size_t flags) -> Status
    {

        auto first = begin(), last = end();
        // Find the first which's base_ lesser than it, 
        auto const insert_position = find_if(first, last, [base, size] (MemRegion &region) { 
            return region.end() < base && ;
        });

        if (insert_position == last) {

        }
 
        auto const overlaped = ::find_if(insert_position, end(), [base, size] (MemRegion &region) { 
            return !(i.base_ > end || base > i.end_); 
        });

        if (overlaped != list.end()) { // If not independent.
            // the 'section' overlaps with another section which represented by 'pos'.
            overlaped->base_ = min(overlaped->base_, base);
            overlaped->end_ = max(overlaped->end_, end);
        } else {
            insert(insert_pos, section);
        }
    }

    auto MemRegionList::remove(size_t base, size_t size) -> Status
    {
    }

    auto MemRegionList::isolate(size_t base, size_t size) -> Status
    {}

    auto MemRegionList::contains(size_t *base, size_t *size) -> ContainsResult 
    {
        auto base_val = *base, size_val = *size;
        for (auto i = 0; i < nr_regions_; ++i) {
            if (list_[i].base < base_val) {
                if (size_val + (base_val - list_[i].base) < list_[i].size) {
                    return ContainsResult::Included;
                } else if (i + 1 < nr_regions_) {
                }
            }
        } 

        return ContainsResult::Exclusive;
    }

    // struct MemBlock:Cursor
    // {
    //     typedef Cursor  Self;

    //     Cursor(MemBlock const *alloc);

    //     auto consume() -> void;

    //     auto move_next() -> ustl::Option<MemRegion>;

    //     auto move_prev() -> ustl::Option<MemRegion>;

    //     template <typename I1, typename I2>
    //     static auto move_next_impl(I1 &, I1, I2 &, I2) -> ustl::Option<MemRegion>;

    // private:
    //     MemBlock *alloc_;
    //     Iter normal_;
    //     Iter reserved_;
    // };

    // MemBlock::Cursor::Cursor(MemBlock const *alloc)
    //     : alloc_(const_cast<MemBlock *>(alloc)),
    //       normal_(alloc_->normal_list_.end()),
    //       reserved_(alloc_->reserved_list_.end())
    // {
    //     if (alloc_->policy_ == Policy::BottomUp) {
    //         normal_ = alloc_->normal_list_.begin();
    //         reserved_ = alloc_->reserved_list_.begin();
    //     }
    // }

    // template <typename I1, typename I2>
    // auto MemBlock::Cursor::move_next_impl(I1 &first1, I1 last1, I2 &first2, I2 last2)
    //     -> ustl::Option<MemRegion>
    // {
    //     auto const smallest = first2, largest = last2;
    //     for (; first1 != last1; ++first1) {
    //         auto const [n_base, n_end, n_flags] = *first1; // Available section.
    //         for (; first2 != last2; ++first2) {
    //             // Allocated section.
    //             auto const r_base = first2 == smallest ? LowerLimit : first2->end_;
    //             auto const r_end = first2 == largest ? UpperLimit : first2->base_;

    //             // 
    //             if (r_base > n_end) { // Sync index.
    //                 break;
    //             } else if (n_base < r_end) {
    //                 ++first1; // For next iteration.
    //                 return {{ max(n_base, r_base), min(n_end, r_end), n_flags }};
    //             }
    //         }
    //     }

    //     return ustl::None;
    // }

    // inline auto MemBlock::Cursor::move_next() -> ustl::Option<MemRegion>
    // {
    //     auto rev1 = RevIter(normal_), rev2 = RevIter(reserved_);
    //     auto res = Self::move_next_impl(
    //         rev1,
    //         alloc_->normal_list_.rend(),
    //         rev2,
    //         alloc_->reserved_list_.rend()
    //     );

    //     normal_ = rev1.base();
    //     reserved_ = rev2.base();

    //     return res;
    // }

    // inline auto MemBlock::Cursor::move_prev() -> ustl::Option<MemRegion> 
    // {
    //     return Self::move_next_impl(
    //         normal_, 
    //         alloc_->normal_list_.end(),
    //         reserved_,
    //         alloc_->reserved_list_.end()
    //     );
    // }

    // MemBlock::MemBlock()
    //     : policy_(Policy::TopDown)
    // {}

    // auto MemBlock::merge(RegionList &list) -> void
    // {
    //     auto n = list.size() - 1;
    //     for (auto i = 0; i < n;) {
    //         auto &[base1, end1, flags1] = list[i];
    //         auto &[base2, end2, flags2] = list[i + 1];
    //         if (end1 + 1 < base2 || flags1 != flags2) {
    //             i += 1;
    //             continue;
    //         }

    //         n -= 1;
    //         base2 = base1;
    //         list.erase(list.begin() + i);
    //     }
    // }

    // /// @brief Insert a new section and ensure it's order. 
    // /// @param list
    // /// @param section
    // auto MemBlock::insert_and_merge(RegionList &list, MemRegion const &section) -> void
    // {
    //     auto const [base, end, flags] = section;

    //     // Find the first which's base_ lesser than it, 
    //     auto const greater = [end, base] (MemRegion &i) { 
    //         return i.base_ >= end; 
    //     };

    //     auto const insert_pos = ::find_if(list.begin(), list.end(), greater);

    //     auto const matcher = [end, base] (MemRegion &i) { 
    //         return !(i.base_ > end || base > i.end_); 
    //     };

    //     auto const overlaped = ::find_if(insert_pos, list.end(), matcher);

    //     if (overlaped != list.end()) { // If not independent.
    //         // the 'section' overlaps with another section which represented by 'pos'.
    //         overlaped->base_ = min(overlaped->base_, base);
    //         overlaped->end_ = max(overlaped->end_, end);
    //     } else {
    //         list.insert(insert_pos, section);
    //     }

    //     // Finally, ... 
    //     Self::merge(list);
    // }

    // /// @brief 
    // /// @param list
    // /// @param range
    // /// @return [start, end)
    // auto MemBlock::isolate(RegionList &list, gktl::Range<PhysAddr> const &range) 
    //     -> ustl::Pair<Iter, Iter>
    // {
    //     auto const range_start = range.base_.as_usize();
    //     auto const range_end = range.end_.as_usize();

    //     auto return_start = Iter(), return_end = Iter();
    //     auto first = list.begin(), last = list.end();
    //     for (; first != last; ++first) {
    //         auto [base, end, flags] = *first;
    //         if (base >= range_end) {
    //             // 'first' is independent of 'range'
    //             break;
    //         } else if (end <= range_start) {
    //             // 'first' is independent of 'range'
    //             continue;
    //         } else if (base < range_start) {
    //             // Split to [base ... range_start) and [range_start ... ),
    //             // then inserts former before `first` .
    //             first->base_ = range_start;
    //             list.insert(first, { base, range_start - 1 });
    //             last = list.end();
    //         } else if (end > range_end) {
    //             // Split to [base ... range_end) and [range_end ... ),
    //             // then inserts former before `first` .
    //             first->base_ = range_end + 1;
    //             list.insert(first--, { base, range_end });
    //             last = list.end();
    //         } else {
    //             // Inside or embeded. 
    //             if (return_start == Iter()) {
    //                 return_start = first;
    //             }
    //             return_end = first;
    //         }
    //     }

    //     return { return_start, ++return_end };
    // }

    // auto MemBlock::find(Layout const &layout, gktl::Range<PhysAddr> bounds) -> ustl::Option<MemRegion>
    // {
    //     auto const lower_limit = bounds.base_.as_usize();
    //     auto const upper_limit = bounds.end_.as_usize();

    //     Cursor cursor{this};
    //     while (auto i = cursor.move_next()) {
    //         auto end = clamp(i->end_, lower_limit, upper_limit);
    //         if (end < layout.size_) {
    //             continue;
    //         }

    //         auto base = align_down(end - layout.size_ + 1, layout.align_);
    //         base = clamp(base, lower_limit, upper_limit);

    //         if (base < end && end - base + 1 >= layout.size_) {
    //             return {{ base, base + layout.size_ - 1, i->flags_ }};
    //         }
    //     }

    //     return ustl::None;
    // }

    // auto MemBlock::allocate_bounded(Layout const &layout, gktl::Range<PhysAddr> const &bounds) -> void *
    // {
    //     DEBUG_ASSERT(reserved_list_.size() < reserved_list_.capacity());

    //     if (auto i = this->find(layout, bounds)) {
    //         decltype(auto) to_reserve = i.value();
    //         this->protect(to_reserve);
    //         return reinterpret_cast<void *>(to_reserve.base_);
    //     } 

    //     return nullptr;
    // }

    // auto MemBlock::deallocate(void *raw, Layout const &layout) -> void
    // {
    //     auto adr = static_cast<char *>(raw);
    //     auto const [first, last] = Self::isolate(this->reserved_list_, gktl::Range<PhysAddr>{adr, adr + layout.size_ - 1});
    //     if (first != last) {
    //         // for_each(reserved_list_.begin(), reserved_list_.end(), 
    //         // [] (MemRegion &i) { Log::debug("{0x%x, 0x%x}\n", i.base_, i.end_); });
    //         log::debug("Deallocate to remove %d sections\n", distance(first, last));
    //         this->reserved_list_.erase(first, last);
    //     }
    // }

    // auto MemBlock::dump() const -> void
    // {
    //     Cursor cursor(this);
    //     log::info("Available memory sections: {}\n", this->normal_list_.size());
    //     // auto const m = this->normal_list_.size();
    //     // for (auto i = 0; i < m; ++i) {
    //     //     auto [base, end, flags] = this->normal_list_[i];
    //     //     Log::info("   {%x, %x, %x}\n", base, end, end - base);
    //     // }
    //     while (auto i = cursor.move_next()) {
    //         auto [base, end, flags] = i.value();
    //         log::info("   {%x, %x}\n", base, end);
    //     }

    //     log::info("Reserved memory sections: {}\n", this->reserved_list_.size());
    //     auto const n = this->reserved_list_.size();
    //     for (auto i = 0; i < n; ++i) {
    //         auto [base, end, flags] = this->reserved_list_[i];
    //         log::info("   {%x, %x}\n", base, end);
    //     }
    // }

} // namespace allocator