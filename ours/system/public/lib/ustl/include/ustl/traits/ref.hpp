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

#ifndef USTL_TRAITS_REF_HPP
#define USTL_TRAITS_REF_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename T>
    using IsRef = std::is_reference<T>;

    template <typename T>
    static auto const IsRefV = std::is_reference_v<T>;

    template <typename T>
    using RemoveRefT = std::remove_reference_t<T>;

    template <typename T>
    using RemoveCvRefT = std::remove_cvref_t<T>;

    template <typename T>
    using AddLValueRefT = std::add_lvalue_reference_t<T>;

    template <typename T>
    using AddRValueRefT = std::add_rvalue_reference_t<T>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_REF_HPP