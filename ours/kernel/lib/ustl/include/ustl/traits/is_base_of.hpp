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

#ifndef USTL_TRAITS_IS_BASE_OF_HPP
#define USTL_TRAITS_IS_BASE_OF_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename Base, typename Derived>
    constexpr bool IsBaseOfV = ::std::is_base_of_v<Base, Derived>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_IS_BASE_OF_HPP