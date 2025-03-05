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

#ifndef USTL_VIEWS_STRING_VIEW_HPP
#define USTL_VIEWS_STRING_VIEW_HPP 1

#include <string_view>

namespace ustl::views {
    template <typename T>
    using BasicStringView = std::basic_string_view<T>;

    typedef BasicStringView<char>   StringView;

    typedef BasicStringView<wchar_t>   WStringView;

} // namespace ustl::views

#endif // #ifndef USTL_VIEWS_STRING_VIEW_HPP