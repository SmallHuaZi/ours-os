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

#ifndef USTL_FMT_DETAILS_FORMATTER_INTEGER_HPP
#define USTL_FMT_DETAILS_FORMATTER_INTEGER_HPP 1

#include <ustl/config.hpp>
#include <ustl/fmt/formatter.hpp>

namespace ustl::fmt {
namespace details {
    template <typename T, typename Char>
    struct FormatterInteger
    {
        template <typename ParseContext>
        auto parse(ParseContext &) const -> ParseContext::IterMut;

        template <typename FormatContext>
        auto format(T, FormatContext &) const -> FormatContext ::IterMut;

        template <typename FormatContext>
        auto formatted_size(T, FormatContext &) const -> FormatContext::IterMut;
    };

} // namespace ustl::fmt::details 

    template <typename Char>
    class Formatter<i8, Char>: public details::FormatterInteger<i8, Char> {};

    template <typename Char>
    class Formatter<i16, Char>: public details::FormatterInteger<i16, Char> {};

    template <typename Char>
    class Formatter<i32, Char>: public details::FormatterInteger<i32, Char> {};

    template <typename Char>
    class Formatter<i64, Char>: public details::FormatterInteger<i64, Char> {};

    template <typename Char>
    class Formatter<u8, Char>: public details::FormatterInteger<u8, Char> {};

    template <typename Char>
    class Formatter<u16, Char>: public details::FormatterInteger<u16, Char> {};

    template <typename Char>
    class Formatter<u32, Char>: public details::FormatterInteger<u32, Char> {};

    template <typename Char>
    class Formatter<u64, Char>: public details::FormatterInteger<u64, Char> {};
}

#include <ustl/fmt/details/formatter_integer.tcc>

#endif // #ifndef USTL_FMT_DETAILS_FORMATTER_INTEGER_HPP