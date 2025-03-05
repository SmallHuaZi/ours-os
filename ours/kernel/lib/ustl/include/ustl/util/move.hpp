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

#ifndef USTL_UTIL_MOVE_HPP
#define USTL_UTIL_MOVE_HPP 1

#include <ustl/config.hpp>
#include <ustl/traits/ref.hpp>

namespace ustl {
    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto move(T &&lr) USTL_NOEXCEPT -> traits::RemoveRefT<T> &&
    { return static_cast<typename traits::RemoveRefT<T> &&>(lr); }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto forward(traits::RemoveRefT<T> &lr) USTL_NOEXCEPT -> T &&
    { return static_cast<T &&>(lr); }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto forward(traits::RemoveRefT<T> &&rr) USTL_NOEXCEPT -> T &&
    { return static_cast<T &&>(rr); }   
        
} // namespace ustl

#endif // #ifndef USTL_UTIL_MOVE_HPP
