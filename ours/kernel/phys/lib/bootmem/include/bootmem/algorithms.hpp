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
#ifndef BOOTMEM_ALGORITHMS_HPP
#define BOOTMEM_ALGORITHMS_HPP 1

#include <bootmem/types.hpp>
#include <bootmem/region.hpp>

#include <ustl/option.hpp>
#include <ustl/util/pair.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/iterator/traits.hpp>
#include <ustl/iterator/reverse_iterator.hpp>

namespace bootmem {
    using ustl::algorithms::max;
    using ustl::algorithms::min;

    template <typename Iterator>
    static auto const IsRegionIteratorV = ustl::traits::IsSameV<ustl::iterator::ElementOfT<Iterator>, Region>;

    template <typename Iterator1, typename Iterator2, typename F>
        CXX20_REQUIRES(ustl::traits::IsInvocableV<F, PhysAddr, PhysAddr, RegionFlags> &&
                       IsRegionIteratorV<Iterator1> && 
                       IsRegionIteratorV<Iterator2> )
    auto find_free_region_if(Iterator1 const &first1, Iterator1 const &last1, // All
                               Iterator2 const &first2, Iterator2 const &last2, // Used
                               F &&f) -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>> 
    {
        while (first1 != last1) {
            PhysAddr last_commit_end = first1->base;
            PhysAddr const start1 = first1->base;
            PhysAddr const end1 = start1 + first1->size;

            while (first2 != last2) {
                PhysAddr const start2 = first2->base;
                PhysAddr const end2 = start2 + first2->size;

                if (end1 <= start2) {
                    ++first1;
                    break;
                }

                if (end2 <= start1 || last_commit_end == start2) {
                    ++first2;
                    continue;
                }

                if (f(last_commit_end, start2, first1->flags)) {
                    return ustl::make_pair(last_commit_end, start2);
                }
                last_commit_end = end2;
            }
        }

        return ustl::none();
    }

    template <typename Iterator1, typename Iterator2, typename F>
        CXX20_REQUIRES(ustl::traits::IsInvocableV<F, PhysAddr, PhysAddr, RegionFlags> &&
                       IsRegionIteratorV<Iterator1> && 
                       IsRegionIteratorV<Iterator2> )
    auto for_each_free_region(Iterator1 const &first1, Iterator1 const &last1, // All
                                Iterator2 const &first2, Iterator2 const &last2, // Used
                                F &&f) -> void
    {
        for (; first1 != last1; ++first1) {
            PhysAddr last_commit_end = first1->base;
            PhysAddr const start1 = first1->base;
            PhysAddr const end1 = start1 + first1->size;

            for (; first2 != last2; ++first2) {
                PhysAddr const start2 = first2->base;
                PhysAddr const end2 = start2 + first2->size;

                if (end1 <= start2) {
                    break;
                }

                if (end2 <= start1 || start2 == last_commit_end) {
                    continue;
                }

                f(last_commit_end, start2, first1->flags);
                last_commit_end = end2;
            }
        }
    }

    template <typename Iterator1, typename Iterator2>
        CXX20_REQUIRES(IsRegionIteratorV<Iterator1> && IsRegionIteratorV<Iterator2>)
    auto lookup_next_free_region(Iterator1 const &first1, Iterator1 const &last1, // All
                                   Iterator2 const &first2, Iterator2 const &last2, // Used
                                   Iterator1 &iter1, Iterator2 &iter2)  -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>>
    {
        for (; iter1 != last1; ++iter1) {
            PhysAddr const start = iter1->base;
            PhysAddr const end = start + iter1->size;
            
            while (true) {
                PhysAddr gap_start; 
                if (iter2 == first2) {
                    gap_start = first1->base;
                } else {
                    auto const prev_iter2 = (iter2 - 1);
                    gap_start = prev_iter2->base + prev_iter2->size;
                }

                PhysAddr gap_end;
                if (iter2 == last2) {
                    auto const prev_last1 = (last1 - 1);
                    gap_end = prev_last1->base + prev_last1->size;
                } else {
                    gap_end = iter2->base; 
                }

                if (gap_start >= end) {
                    break;
                }

                // The two regions has intersected with each other. 
                if (start < gap_end) {
                    // Advance one step for next iteration.
                    if (end <= gap_end) {
                        ++iter1;
                    } else if (iter2 != last2) {
                        ++iter2;
                    }
                    return ustl::make_pair(max(start, gap_start), min(end, gap_end));
                }

                if (iter2 == last2) {
                    break;
                }
                ++iter2;
            }
        }

        return ustl::none();
    }

    template <typename Iterator1, typename Iterator2>
        CXX20_REQUIRES(IsRegionIteratorV<Iterator1> && IsRegionIteratorV<Iterator2>)
    auto lookup_prev_free_region(Iterator1 const &first1, Iterator1 const &last1, // All
                                   Iterator2 const &first2, Iterator2 const &last2, // Used
                                   Iterator1 &iter1, Iterator2 &iter2)  -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>>
    {
        // for (auto i = iter1; i != first1; --i) {
        for (; iter1 != first1; --iter1) {
            PhysAddr const start = (iter1 - 1)->base;
            PhysAddr const end = start + (iter1 - 1)->size;

            while (true) {
                // The range [gap_start, gap_end) represents the gap between the previous
                // and current  reserved region. It must be free or not existent.
                PhysAddr gap_start; 
                if (iter2 == first2) {
                    // Minimal address
                    gap_start = first1->base;
                } else {
                    // The end of previous reserved region
                    auto const prev_iter2 = (iter2 - 1);
                    gap_start = prev_iter2->base + prev_iter2->size;
                }

                PhysAddr gap_end;
                if (iter2 == last2) {
                    // Maximal address
                    auto const prev_last1 = (last1 - 1);
                    gap_end = prev_last1->base + prev_last1->size;
                } else {
                    gap_end = iter2->base; 
                }

                if (gap_end <= start) {
                    break;
                }

                // The two regions has intersected with each other. 
                if (end > gap_start) {
                    // Advance one step for next iteration.
                    if (start >= gap_start) {
                        --iter1;
                    } else {
                        --iter2;
                    }
                    return ustl::make_pair(max(start, gap_start), min(end, gap_end));
                }

                if (iter2 == first2) {
                    break;
                }

                --iter2;
            }
        }

        return ustl::none();
    }

    template <typename Out, typename Iterator>
        CXX20_REQUIRES(IsRegionIteratorV<Iterator>)
    auto format_to(Out &&out, Iterator &&first, Iterator &&last) -> void
    {
        out("| {:<16} | {:<18} | {}", "Type", "Base", "Size");
        while (first != last) {
            out("| {:<16} | 0x{:<16X} | 0x{:<16X}", to_string(first->type()), first->base, first->size);
        }
    }

} // namespace bootmem

#endif // #ifndef BOOTMEM_ALGORITHMS_HPP