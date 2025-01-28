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

#ifndef USTL_FMT_DETAILS_FORMATTER_CSTRING_HPP
#define USTL_FMT_DETAILS_FORMATTER_CSTRING_HPP 1

#include <ustl/fmt/formatter.hpp>

namespace ustl::fmt {
    template <typename Char>
    struct Formatter<char const *, Char>
    {
        template <typename ParseContext>
        auto parse(ParseContext &) const -> ParseContext::IterMut
        {  return {};  }

        template <typename FormatContext>
        auto format(char const *, FormatContext &) const -> FormatContext::IterMut
        {    }

        template <typename FormatContext>
        auto formatted_size(char const *, FormatContext &) const -> usize 
        {  return {};  }
    };

} // namespace ustl::fmt 

#endif // #ifndef USTL_FMT_DETAILS_FORMATTER_CSTRING_HPP