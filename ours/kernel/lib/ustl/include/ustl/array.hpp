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
#ifndef USTL_ARRAY_HPP
#define USTL_ARRAY_HPP 1

#include <array>

namespace ustl {
    template <typename T, int... Ns>
    struct MakeArray;

    template <typename T, int N>
    struct MakeArray<T, N> {
        typedef std::array<T, N>  Type;
    };

    // Like T array[N][Ns]...
    template <typename T, int N, int... Ns>
    struct MakeArray<T, N, Ns...> {
        typedef std::array<typename MakeArray<T, Ns...>::Type, N> Type;
    };

    template <typename T, int N, int... Ns>
    using Array = typename MakeArray<T, N, Ns...>::Type;

} // namespace ustl

#endif // #ifndef USTL_ARRAY_HPP