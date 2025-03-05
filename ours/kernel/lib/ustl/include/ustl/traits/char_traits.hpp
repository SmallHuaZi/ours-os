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

#ifndef USTL_TRAITS_CHAR_TRAITS_HPP
#define USTL_TRAITS_CHAR_TRAITS_HPP 1

#include <string>

namespace ustl::traits {
    template <typename Char>
    using CharTraits = std::char_traits<Char>;

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_CHAR_TRAITS_HPP