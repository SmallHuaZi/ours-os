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

#ifndef USTL_TRAITS_INVOKABLE_HPP
#define USTL_TRAITS_INVOKABLE_HPP 1

#include <concepts>
#include <type_traits>

namespace ustl::traits {
    template <typename Fn, typename... Args> 
    concept Invocable = std::invocable<Fn, Args...>;

    template <typename Fn, typename... Args>
    constexpr bool IsInvocableV = std::is_invocable_v<Fn, Args...>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_INVOKABLE_HPP