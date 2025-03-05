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

#ifndef USTL_ITERATOR_ACCESS_HPP
#define USTL_ITERATOR_ACCESS_HPP 1

#include <ustl/config.hpp>

namespace ustl::iter {
    template <typename T, usize N>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto begin(T (&array)[N]) USTL_NOEXCEPT -> T *
    {  return array;  }

    template <typename T, usize N>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto end(T (&array)[N]) USTL_NOEXCEPT -> T *
    {  return array + N;  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto begin(T& c) USTL_NOEXCEPT -> decltype(c.begin())
    {  return c.begin();  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto begin(T const &c) USTL_NOEXCEPT -> decltype(c.begin())
    {  return c.begin();  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto end(T &c) USTL_NOEXCEPT -> decltype(c.end())
    {  return c.end();  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto end(T const &c) USTL_NOEXCEPT -> decltype(c.end())
    {  return c.end();  }

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_ACCESS_HPP