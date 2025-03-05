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

#ifndef USTL_UTIL_FIXED_STRING_HPP
#define USTL_UTIL_FIXED_STRING_HPP 1

#include <ustl/config.hpp>

namespace ustl::util {
    // template <usize N>
    // struct FixedString
    // {
    //     USTL_CONSTEXPR 
    //     FixedString(char const (&s)[N])
    //         : pstr(s)
    //     {}

    //     template <usize M>
    //     FixedString(FixedString<M> const &other)
    //         : pstr(other.pstr)
    //     {  static_assert(M < N, "");  }

    //     char const *pstr;
    // };

    template <usize N>
    struct FixedString
    {
        USTL_CONSTEXPR 
        FixedString(char const (&s)[N])
        {
            for (auto i = 0; i < N; ++i) {
                str_[i] = s[i];
            }
        }

        template <usize M>
        USTL_CONSTEXPR 
        FixedString(FixedString<M> const &other)
        {
            static_assert(M < N, "");
            auto i = 0;
            for (; i < N; ++i) {
                str_[i] = other.str_[i];
            }
            str_[i + 1] = 0;
        }

        USTL_CONSTEXPR
        auto data() const -> char const *
        {  return str_;  }

        USTL_CONSTEXPR
        static auto size() -> usize 
        {  return N;  }

        // char str_[N];
        char str_[N];
    };

    /// CTAD
    template <usize N>
    FixedString(char const (*)[N]) -> FixedString<N>;

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_FIXED_STRING_HPP