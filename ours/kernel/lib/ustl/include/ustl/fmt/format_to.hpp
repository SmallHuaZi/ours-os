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

#ifndef USTL_FMT_FORMAT_TO_HPP
#define USTL_FMT_FORMAT_TO_HPP 1

/// Mainly provides the following interfaces:
///     1) auto vformat_to(Iterator, views::StringView, FormatArgs) -> Iterator
///     2) auto format_to(Iterator, views::StringView, Args&&...) -> Iterator
///     3) auto formatted_size(views::StringView, Args&&...) -> Iterator
///
/// And, user should use the `format_to()` to 

#include <ustl/fmt/details/buffer_format_to.hpp>
#include <ustl/fmt/details/basic_format_string.hpp>
#include <ustl/fmt/details/do_vformat_to.hpp>
#include <ustl/fmt/make_format_args.hpp>

namespace ustl::fmt {
    template <typename Iterator>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto vformat_to(Iterator out, views::StringView fmt, FormatArgs args) USTL_NOEXCEPT 
        -> Result<Iterator>
    {
        return USTL_VFMT do_vformat_to(ustl::move(out), fmt, args);
    }

    template <typename Iterator>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto vformat_to(Iterator out, views::WStringView fmt, WFormatArgs args) USTL_NOEXCEPT 
        -> Result<Iterator>
    {
        return USTL_VFMT do_vformat_to(ustl::move(out), fmt, args);
    }

    template <typename Iterator, typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto format_to(Iterator out, FormatString<Args...> fmt, Args&&... args) USTL_NOEXCEPT 
        -> Result<Iterator>
    {
        return USTL_FMT vformat_to(ustl::move(out), fmt.get(), make_format_args(args...));
    }

    template <typename Iterator, typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto format_to(Iterator out, WFormatString<Args...> fmt, Args &&...args) USTL_NOEXCEPT 
        -> Result<Iterator>
    {
        return USTL_FMT vformat_to(ustl::move(out), fmt.get(), make_wformat_args(args...));
    }

    template <typename Context, typename Iterator, typename Char>
    auto vformat_to_n(
        Iterator out, 
        iter::IterDistanceOfT<Iterator> n,
        views::BasicStringView<Char> fmt, 
        BasicFormatArgs<Context> args) USTL_NOEXCEPT
    {
        details::FormatToNBuffer<Iterator, Char> buffer(ustl::move(out), n);
        vformat_to(BasicFormatParseContext{fmt, args.size()},
                  create_format_context(buffer.make_output_iterator(), args));
        return ustl::move(buffer).result();
    }

    template <typename Iterator, typename... Args>
    auto format_to_n(
        Iterator out, 
        iter::IterDistanceOfT<Iterator> n,
        FormatString<Args...> fmt, 
        Args &&...args) USTL_NOEXCEPT
    {
        return vformat_to_n<FormatContext>(
            ustl::move(out), 
            n, 
            fmt.get(),
            make_format_args(args...)
        );
    }

    template <typename Iterator, typename... Args>
    auto format_to_n(
        Iterator out, 
        iter::IterDistanceOfT<Iterator> n,
        WFormatString<Args...> fmt, 
        Args &&...args) USTL_NOEXCEPT
    {
        return vformat_to_n<WFormatContext>(
            ustl::move(out), 
            n, 
            fmt.get(),
            make_wformat_args(args...)
        );
    }

}

#endif // #ifndef USTL_FMT_FORMAT_TO_HPP