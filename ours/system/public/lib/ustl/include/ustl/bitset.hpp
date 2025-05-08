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
#ifndef USTL_BITSET_HPP
#define USTL_BITSET_HPP 1

#include <bitset>
#include <ustl/config.hpp>
#include <ustl/limits.hpp>

namespace ustl {
    template <int NBits>
    struct BitSet: public ::std::bitset<NBits> {
        typedef ::std::bitset<NBits>        Base;
        using Base::Base;

        auto clear() -> void {
            for (auto i = 0; i < NBits; ++i) {
                this->Base::set(i, 0);
            }
        }

        auto test_range(usize start, usize num_bits) -> bool {
            bool v = true;
            for (auto i = 0; i < num_bits; ++i) {
                v &= this->test(i + start);
            }
            return v;
        }
    };

    template <usize Delta = 0, typename BitSet, typename Integer>
    auto copy_bits(BitSet &bitset, Integer value) -> void {
        for (auto i = 0; i < ustl::NumericLimits<Integer>::DIGITS; ++i) {
            if (((1 << i) & value) != 0) {
                bitset.set(Delta + i);
            }
        }
    }

} // namespace ustl

#endif // #ifndef USTL_BITSET_HPP