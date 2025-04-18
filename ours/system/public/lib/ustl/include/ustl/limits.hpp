// ustl USTL_LIMITS_HPP
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
#ifndef USTL_LIMITS_HPP
#define USTL_LIMITS_HPP 1

#include <limits>

namespace ustl {
    template <typename T>
    struct NumericLimits 
        : public ::std::numeric_limits<T>
    {
        typedef ::std::numeric_limits<T>    Base;

        constexpr static auto DIGITS = Base::digits;
        constexpr static auto DIGITS10 = Base::digits10;
    };

} // namespace ustl

#endif // #ifndef USTL_LIMITS_HPP