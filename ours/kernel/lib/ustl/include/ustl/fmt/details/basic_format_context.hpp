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

#ifndef USTL_FMT_DETAILS_BASIC_FORMAT_CONTEXT_HPP
#define USTL_FMT_DETAILS_BASIC_FORMAT_CONTEXT_HPP 1

#include <ustl/fmt/details/fwd.hpp>
#include <ustl/fmt/details/buffer_output.hpp>
#include <ustl/fmt/details/basic_format_arg.hpp>

#include <ustl/option.hpp>
#include <ustl/iterator/back_insert_iterator.hpp>

namespace ustl::fmt::details {
    template <typename Iterator, typename CharType>
    class BasicFormatContext
    {
        typedef BasicFormatArg<BasicFormatContext>      FormatArg;
        typedef BasicFormatArgs<BasicFormatContext>     FormatArgs;

    public:
        typedef CharType    Char;
        typedef Iterator    IterMut;

        auto arg(usize id) const USTL_NOEXCEPT -> FormatArg
        {  return args_.get(id);  }

        auto out() -> IterMut 
        { return ustl::move(out_it_); }

        auto advance_to(IterMut it) -> void 
        { out_it_ = ustl::move(it); }

        /// Helper routine to create a BasicFormatContext.
        /// This is needed since the constructor is private.
        template <typename Locale = int>
        friend auto create_format_context(
            Iterator out, 
            BasicFormatArgsRebind<Iterator, Char> args, 
            Option<Locale> && = {})
            -> BasicFormatContext
        {  return BasicFormatContext(ustl::move(out), args);  }

    private:
        // Note: the Standard doesn't specify the required constructors.
        explicit BasicFormatContext(IterMut out_it, FormatArgs args)
            : out_it_(ustl::move(out_it)), args_(args) 
        {}

    private:
        IterMut out_it_;
        FormatArgs args_;
    };

}

#endif // #ifndef USTL_FMT_DETAILS_BASIC_FORMAT_CONTEXT_HPP