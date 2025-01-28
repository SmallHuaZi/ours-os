// ustl/fmt USTL/FMT_FORMATTER_HPP
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

#ifndef USTL_FMT_FORMATTER_HPP
#define USTL_FMT_FORMATTER_HPP 1

#include <ustl/config.hpp>

namespace ustl::fmt {

    /// The default `Formatter` is a unconstructible class, user need 
    /// to implement a specialization about it for user-defined object.
    ///
    /// But don't worry, some object in `ustl` has been implemented 
    /// its `Formatter` by default.
    template <typename T, typename Char = char>
    class Formatter
    {
        typedef Formatter       Self;

    public:
        Formatter() = delete;
        Formatter(Self const &) = delete;
        auto operator=(Self const &) -> Self & = delete;

        // Unimplement and required.
        template <typename ParseContext>
        auto parse(ParseContext &) const -> ParseContext::IterMut = delete;

        // Unimplement and required.
        // Requires:
        //  1) No throws.
        template <typename FormatContext>
        auto format(T, FormatContext &) const -> FormatContext::IterMut = delete;

        // Unimplement but optional.
        template <typename FormatContext>
        auto formatted_size(T, FormatContext &) const -> usize = delete;
    };

} //namespace ustl::fmt

#endif // #ifndef USTL_FMT_FORMATTER_HPP