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
#include <ustl/traits/underlying.hpp>

#define USTL_ENABLE_ENUM_BITS(T)    \
USTL_FORCEINLINE USTL_CONSTEXPR     \
auto operator|(T a, T b) -> T       \
{\
    typedef ustl::traits::UnderlyingTypeT<T>  U;\
    return static_cast<T>(static_cast<U>(a) | static_cast<U>(b));\
}\
USTL_FORCEINLINE USTL_CONSTEXPR \
auto operator&(T a, T b) -> T\
{\
    typedef ustl::traits::UnderlyingTypeT<T>  U;\
    return static_cast<T>(static_cast<U>(a) & static_cast<U>(b));\
}\
USTL_FORCEINLINE USTL_CONSTEXPR \
auto operator^(T a, T b) -> T\
{\
    typedef ustl::traits::UnderlyingTypeT<T>  U;\
    return static_cast<T>(static_cast<U>(a) ^ static_cast<U>(b));\
}\
USTL_FORCEINLINE USTL_CONSTEXPR \
auto operator|=(T &a, T b) -> T &\
{\
    a = a | b;\
    return a;\
}\
USTL_FORCEINLINE USTL_CONSTEXPR \
auto operator&=(T &a, T b) -> T &\
{\
    a = a & b;\
    return a;\
}\
USTL_FORCEINLINE USTL_CONSTEXPR \
auto operator^=(T &a, T b) -> T &\
{\
    a = a ^ b;\
    return a;\
}\

#endif // #ifndef USTL_UTIL_ENUM_BITS_HPP