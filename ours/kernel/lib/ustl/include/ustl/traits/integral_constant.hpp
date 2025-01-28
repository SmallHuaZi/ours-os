// ustl/traits USTL/TRAITS_INTEGRAL_CONSTANT_HPP
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

#ifndef USTL_TRAITS_INTEGRAL_CONSTANT_HPP
#define USTL_TRAITS_INTEGRAL_CONSTANT_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename T, T V>
    using IntegralConstant = std::integral_constant<T, V>;

    template <bool V>
    using BoolConstant = std::bool_constant<V>;
    using TrueType = std::true_type;
    using FalseType = std::false_type;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_INTEGRAL_CONSTANT_HPP