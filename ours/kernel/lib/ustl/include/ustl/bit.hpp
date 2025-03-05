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
#include <ustl/traits/integral.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace ustl {
    using std::bit_ceil;
    using std::bit_floor;
    using std::bit_width;

    template <typename Int, Int StartBit, Int Size>
    struct MakeBitMask
        : public traits::IntegralConstant<Int, ((Int(1) << Size) - 1) << StartBit>
    {  static_assert(traits::IsIntegralV<Int>, "`Int` is not a valid integer");  };

    template <typename Int, Int StartBit, Int Size>
    auto make_bitmask() -> Int
    {  return MakeBitMask<Int, StartBit, Size>::VALUE;  }

} // namespace ustl

#endif // #ifndef USTL_BIT_HPP