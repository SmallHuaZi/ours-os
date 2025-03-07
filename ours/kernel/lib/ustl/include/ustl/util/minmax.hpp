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

#ifndef USTL_UTIL_MINMAX_HPP
#define USTL_UTIL_MINMAX_HPP 1

#include <ustl/traits/integral_constant.hpp>

namespace ustl {
    template <typename T>
    struct Min
    {
        template<T X, T Y>
        struct Get
            : traits::IntegralConstant<T, (X > Y) ? Y : X>
        {};
    };

    template <typename T>
    struct Max
    {
        template<T X, T Y>
        struct Get
            : traits::IntegralConstant<T, (X < Y) ? Y : X>
        {};
    };

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_MINMAX_HPP