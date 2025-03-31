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
#ifndef USTL_TRAITS_ENUM_HPP
#define USTL_TRAITS_ENUM_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename T>
    using IsEnum = ::std::is_enum<T>;

    template <typename T>
    static auto const IsEnumV = ::std::is_enum_v<T>;

    template <typename T>
    using IsScopeEnum = ::std::is_scoped_enum<T>;

    template <typename T>
    static auto const IsScopeEnumV = ::std::is_scoped_enum_v<T>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_ENUM_HPP