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
#ifndef USTL_CONCEPTS_CONVERTIBLE_TO_HPP
#define USTL_CONCEPTS_CONVERTIBLE_TO_HPP 1

#include <concepts>

namespace ustl::concepts {
    template <typename T, typename U>
    concept ConvertibleTo = std::convertible_to<T, U>;

} // namespace ustl::concepts

#endif // #ifndef USTL_CONCEPTS_CONVERTIBLE_TO_HPP