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
#include <ours/config.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace gktl {
    /// A range is a half-open interval [start, end).
    template <typename T>
    struct Range {
        FORCE_INLINE CXX11_CONSTEXPR
        auto length() const -> ours::usize  {
            return end - start;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_empty() const -> bool {
            return start == end;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto contains(T t) const -> bool {
            return t >= start && t < end;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto overlaps(T other_start, T other_end) const -> bool {
            return (start < other_end) && (end > other_start);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto left_difference_with(Range<T> const &other) const -> Range<T> {
            if (overlaps(other) && start < other.start) {
                return Range<T> { start, other.start };
            }

            // If no valid left difference, return an invalid range (or handle as needed)
            return Range<T> { start, start };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto right_difference_with(Range<T> const &other) const -> Range<T> {
            if (overlaps(other) && end > other.end) {
                return Range<T> { other.end, end };
            }

            // If no valid right difference, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto intersection_with(Range<T> const &other) const -> Range<T> {
            if (overlaps(other)) {
                return Range<T> {
                    ustl::algorithms::max(start, other.start),
                    ustl::algorithms::min(end, other.end)
                };
            }

            // If no valid intersection, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto union_with(Range<T> const &other) const -> Range<T> {
            if (overlaps(other)) {
                return Range<T> {
                    ustl::algorithms::min(start, other.start),
                    ustl::algorithms::max(end, other.end)
                };
            }

            // If no overlap, return an invalid range (or handle as needed)
            return Range<T> { end, end };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto roundup(T b) -> void {
        }

        T start;
        T end;
    };

    template <typename T>
    Range(T start, T end) -> Range<T>;

    template <typename T>
    CXX11_CONSTEXPR
    auto make_range(T start, T end) -> Range<T> {
        return Range<T> { start, end };
    }

} // namespace ours

#endif // #ifndef KTL_RANGE_HPP