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
#ifndef USTL_BIT_HPP
#define USTL_BIT_HPP 1

#include <bit>
#include <ustl/limits.hpp>
#include <ustl/traits/integral.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace ustl {
    using std::bit_ceil;
    using std::bit_floor;
    using std::bit_width;
    using std::popcount;
    using std::countr_one;
    using std::countl_one;
    using std::countr_zero;
    using std::countl_zero;

    template <typename Int, Int StartBit, Int Size>
    struct MakeBitMask {
        static_assert(traits::IsIntegralV<Int>, "`Int` is not a valid integer");
        static_assert(NumericLimits<Int>::DIGITS >= Size, "`Int` is too small");

        USTL_CONSTEXPR
        static Int const VALUE = Size == NumericLimits<Int>::DIGITS ?
                                  ~Int(0) :
                                  ((Int(1) << Size) - Int(1)) << StartBit;
    };

    template <typename Int, Int StartBit, Int Size>
    auto make_bitmask() -> Int
    {  return MakeBitMask<Int, StartBit, Size>::VALUE;  }

    template <typename Int>
    auto make_bitmask(Int start, Int end) -> Int
    {  return ((Int(1) << (end - start)) - 1) << start;  }

} // namespace ustl

#endif // #ifndef USTL_BIT_HPP