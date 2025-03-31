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

#ifndef USTL_TRAITS_CONDITIONAL_HPP
#define USTL_TRAITS_CONDITIONAL_HPP 1

#include <type_traits>

namespace ustl::traits {
    template <bool Cond, typename Then, typename Else>
    using ConditionalT = ::std::conditional_t<Cond, Then, Else>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_CONDITIONAL_HPP