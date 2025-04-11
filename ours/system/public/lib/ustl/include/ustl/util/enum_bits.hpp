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
#ifndef USTL_UTIL_ENUM_BITS_HPP
#define USTL_UTIL_ENUM_BITS_HPP 1

#include <ustl/config.hpp>
#include <ustl/traits/enum.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/enable_if.hpp>
#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/underlying.hpp>
#include <ustl/traits/declval.hpp>

#define USTL_ENABLE_ENUM_BITMASK(T)    \
static auto ustl_enable_enum_bitmask(T) -> ustl::traits::TrueType;\

#define USTL_ENABLE_ENUM_BITMASK_FRIEND(T)    \
friend auto ustl_enable_enum_bitmask(T) -> ustl::traits::TrueType;\

template <typename Enum>
USTL_CONSTEXPR
static auto ustl_enable_enum_bitmask(Enum e) -> ustl::traits::FalseType;

template <typename Enum>
static bool const UstlEnumBitMaskEnabledV = decltype(ustl_enable_enum_bitmask(ustl::traits::declval<Enum>()))();

template <typename Enum>
USTL_FORCEINLINE USTL_CONSTEXPR
auto operator|(Enum lhs, Enum rhs) 
    -> ustl::traits::EnableIfT<UstlEnumBitMaskEnabledV<Enum>, Enum>
{
    typedef ustl::traits::UnderlyingTypeT<Enum>     Underlying;
    return static_cast<Enum> (
        static_cast<Underlying>(lhs) |
        static_cast<Underlying>(rhs)
    );
}

template <typename Enum>
USTL_FORCEINLINE USTL_CONSTEXPR
auto operator&(Enum lhs, Enum rhs) 
    -> ustl::traits::EnableIfT<UstlEnumBitMaskEnabledV<Enum>, Enum>
{
    typedef ustl::traits::UnderlyingTypeT<Enum>     Underlying;
    return static_cast<Enum> (
        static_cast<Underlying>(lhs) &
        static_cast<Underlying>(rhs)
    );
}

template <typename Enum>
USTL_FORCEINLINE USTL_CONSTEXPR
auto operator~(Enum e) 
    -> ustl::traits::EnableIfT<UstlEnumBitMaskEnabledV<Enum>, Enum>
{
    typedef ustl::traits::UnderlyingTypeT<Enum>     Underlying;
    return static_cast<Enum>(~static_cast<Underlying>(e));
}

template<typename Enum>
USTL_FORCEINLINE USTL_CONSTEXPR
auto operator|=(Enum& lhs, Enum rhs) 
    -> ustl::traits::EnableIfT<UstlEnumBitMaskEnabledV<Enum>, ustl::traits::AddLvalRefT<Enum>>
{
    typedef ustl::traits::UnderlyingTypeT<Enum>     Underlying;
    lhs = lhs | rhs;
    return lhs;
}

template<typename Enum>
USTL_FORCEINLINE USTL_CONSTEXPR
auto operator&=(Enum& lhs, Enum rhs) 
    -> ustl::traits::EnableIfT<UstlEnumBitMaskEnabledV<Enum>, ustl::traits::AddLvalRefT<Enum>>
{
    typedef ustl::traits::UnderlyingTypeT<Enum>     Underlying;
    lhs = lhs & rhs;
    return lhs;
}

#endif // #ifndef USTL_UTIL_ENUM_BITS_HPP