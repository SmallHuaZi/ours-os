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

#ifndef USTL_FMT_DETAILS_FWD_HPP
#define USTL_FMT_DETAILS_FWD_HPP 1

#include <ustl/traits/idextity.hpp>
#include <ustl/iterator/back_insert_iterator.hpp>

namespace ustl::fmt {
namespace details {
    /// Internal types
    template <typename Context>
    class BasicFormatArgPayload;

    template <typename Char>
    struct OutputBuffer;

    /// class BasicFormatContext;
    template <typename Iterator, typename Char>
    class BasicFormatContext;

    typedef BasicFormatContext<iter::BackInsertIterator<OutputBuffer<char>>, char>
        FormatContext;

    typedef BasicFormatContext<iter::BackInsertIterator<OutputBuffer<wchar_t>>, wchar_t>
        WFormatContext;

    /// class BasicFormatParseContext;
    template <typename Char>
    class BasicFormatParseContext;

    typedef BasicFormatParseContext<char>       FormatParseContext;
    typedef BasicFormatParseContext<wchar_t>    WFormatParseContext;

    /// class BasicFormatArg
    template <typename Context>
    class BasicFormatArg;

    typedef BasicFormatArg<FormatContext>       FormatArg;
    typedef BasicFormatArg<WFormatContext>      WFormatArg;

    template <typename Iterator, typename Char>
    using BasicFormatArgRebind = BasicFormatArg<BasicFormatContext<Iterator, Char>>;

    /// class BasicFormatArgs
    template <typename Context>
    class BasicFormatArgs;

    typedef BasicFormatArgs<FormatContext>       FormatArgs;
    typedef BasicFormatArgs<WFormatContext>      WFormatArgs;

    template <typename Iterator, typename Char>
    using BasicFormatArgsRebind = BasicFormatArgs<BasicFormatContext<Iterator, Char>>;

    /// FormatString
    template <typename Char, typename... Args>
    class BasicFormatString;

    template <typename... Args>
    using FormatString = BasicFormatString<char, traits::IdextityT<Args>...>;

    template <typename... Args>
    using WFormatString = BasicFormatString<wchar_t, traits::IdextityT<Args>...>;

} // namespace ustl::fmt::details

    using details::BasicFormatContext;
    using details::FormatContext;
    using details::WFormatContext;

    using details::BasicFormatArgs;
    using details::FormatArgs;
    using details::WFormatArgs;

    using details::BasicFormatArg;
    using details::FormatArg;
    using details::WFormatArg;

    using details::BasicFormatParseContext;
    using details::FormatParseContext;
    using details::WFormatParseContext;

    using details::BasicFormatString;
    using details::FormatString;
    using details::WFormatString;

    /// Public types
    template <typename T, typename Char>
    class Formatter;

} // namespace ustl::fmt

#endif // #ifndef USTL_FMT_DETAILS_FWD_HPP