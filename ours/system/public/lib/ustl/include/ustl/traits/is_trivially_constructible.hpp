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
#ifndef USTL_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_HPP
#define USTL_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_HPP 1

#include <type_traits>

namespace ustl::traits {
    template<typename T, typename... Args>
    using IsTrivillyConstructible = std::is_trivially_constructible<T, Args...>;

    template<typename T, typename... Args>
    constexpr bool const IsTrivillyConstructibleV = std::is_trivially_constructible_v<T, Args...>;

    template<typename T>
    using IsTrivillyCopyConstructible = std::is_trivially_copy_constructible<T>;

    template<typename T, typename... Args>
    constexpr bool const IsTrivillyCopyConstructibleV = std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    using IsTrivillyMoveConstructible = std::is_trivially_move_constructible<T>;

    template<typename T, typename... Args>
    constexpr bool const IsTrivillyMoveConstructibleV = std::is_trivially_move_constructible_v<T>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_IS_TRIVILBLE_CONSTRUCTIBLE_HPP