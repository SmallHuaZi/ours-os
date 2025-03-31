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

#ifndef USTL_TRAITS_CV_HPP
#define USTL_TRAITS_CV_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename T>
    using AddConstT = std::add_const_t<T>;

    template <typename T>
    using RemoveConstT = std::remove_const_t<T>;

    template <typename T>
    using AddVolatileT= std::add_volatile_t<T>;

    template <typename T>
    using RemoveVolatileT= std::remove_volatile_t<T>;

    template <typename T>
    using AddCvT = std::add_cv_t<T>;

    template <typename T>
    using RemoveCvT = std::remove_cv_t<T>;

    template <typename T>
    using AddCvRefT = std::remove_cvref_t<T>;

    template <typename T>
    using RemoveCvRefT = std::remove_cvref_t<T>;

    template <typename T>
    constexpr bool IsConstV = std::is_const_v<T>;

    template <typename T>
    constexpr bool IsVolatileV= std::is_volatile_v<T>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_CV_HPP