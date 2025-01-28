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

#ifndef GKTL_RANGE_HPP
#define GKTL_RANGE_HPP 1

#include <ours/types.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace gktl {
    /// A range is a half-open interval [start, end).
    template <typename T>
    struct Range
    {
        constexpr auto length() const -> ours::usize 
        {  return end - start;  }

        constexpr auto is_empty() const -> bool
        {  return start == end;  }

        constexpr auto contains(T t) const -> bool
        {  return t >= start && t < end;  }

        constexpr auto overlaps(Range<T> const &other) const -> bool
        {  return (start < other.end) && (end > other.start); } 

        constexpr auto left_difference_with(Range<T> const &other) const -> Range<T>
        {
            if (overlaps(other) && start < other.start) {
                return Range<T> { start, other.start };
            }

            // If no valid left difference, return an invalid range (or handle as needed)
            return Range<T> { start, start };
        }

        constexpr auto right_difference_with(Range<T> const &other) const -> Range<T>
        {
            if (overlaps(other) && end > other.end) {
                return Range<T> { other.end, end };
            }

            // If no valid right difference, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        constexpr auto intersection_with(Range<T> const &other) const -> Range<T>
        {
            if (overlaps(other)) {
                return Range<T> {
                    ustl::algorithms::max(start, other.start),
                    ustl::algorithms::min(end, other.end)
                };
            }

            // If no valid intersection, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        constexpr auto union_with(Range<T> const &other) const -> Range<T>
        {
            if (overlaps(other)) {
                return Range<T> {
                    ustl::algorithms::min(start, other.start),
                    ustl::algorithms::max(end, other.end)
                };
            }

            // If no overlap, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        T start;
        T end;
    };

    template <typename T>
    constexpr auto make_range(T start, T end) -> Range<T>
    {  return Range<T> { start, end };  }

} // namespace ours 

#endif // #ifndef KTL_RANGE_HPP