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

#ifndef USTL_TRAITS_IS_CONVERTIBLE_HPP
#define USTL_TRAITS_IS_CONVERTIBLE_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename From, typename To>
    constexpr bool IsConvertibleV = std::is_convertible_v<From, To>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_IS_CONVERTIBLE_HPP