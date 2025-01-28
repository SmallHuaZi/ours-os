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

#ifndef USTL_TRAITS_VOID_HPP
#define USTL_TRAITS_VOID_HPP 1

#include <type_traits>
#include <ustl/placeholders.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl::traits {
    template <typename... T>
    using VoidT = ::std::void_t<T...>;

    template <typename T>
    constexpr bool IsVoidV = ::std::is_void_v<T>;

    template <typename T>
    using DevoidT = ConditionalT<IsVoidV<T>, Monostate, T>;


} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_VOID_HPP