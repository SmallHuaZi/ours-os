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

#include <ustl/config.hpp>

namespace ustl::traits {
    /// CRTP interface class, only requires that the derived provides 
    /// a static data member named `VALUE`
    template <typename Derived, typename T>
    struct IntegralConstantInterface
    {
        typedef T         Element;
        typedef Derived   Self;

        USTL_FORCEINLINE USTL_CONSTEXPR
        operator Element() USTL_NOEXCEPT
        {  return Derived::VALUE;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator()() USTL_NOEXCEPT -> Element
        {  return Derived::VALUE;  }
    };

    template <typename T, T V>
    struct IntegralConstant
        : public IntegralConstantInterface<IntegralConstant<T, V>, T>
    {
        USTL_CONSTEXPR 
        static T const VALUE = V;
    };

    template <bool V>
    struct BoolConstant
        : public IntegralConstant<bool, V>
    {};

    typedef BoolConstant<true>       TrueType;
    typedef BoolConstant<false>      FalseType;

    template <isize V>
    struct IntConstant
        : public IntegralConstant<isize, V>
    {};

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_INTEGRAL_CONSTANT_HPP