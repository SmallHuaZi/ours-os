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
#ifndef USTL_TRAITS_PTR_HPP
#define USTL_TRAITS_PTR_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <typename T>
    using IsPtr = std::is_pointer<T>;

    template <typename T>
    static auto const IsPtrV = std::is_pointer_v<T>;

    template <typename T>
    using AddPtr = std::add_pointer<T>;

    template <typename T>
    using AddPtrT = std::add_pointer_t<T>;

    template <typename T>
    using RemovePtr = std::remove_pointer<T>;

    template <typename T>
    using RemovePtrT = std::remove_pointer_t<T>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_PTR_HPP