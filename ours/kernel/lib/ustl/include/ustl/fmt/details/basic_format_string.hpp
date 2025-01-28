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

#ifndef USTL_FMT_DETAILS_BASIC_FORMAT_STRING_HPP
#define USTL_FMT_DETAILS_BASIC_FORMAT_STRING_HPP 1

#include <ustl/views/string_view.hpp>
#include <ustl/collections/array.hpp>

#include <ustl/traits/ref.hpp>
#include <ustl/traits/enable_if.hpp>
#include <ustl/traits/is_convertible.hpp>
#include <ustl/fmt/details/validate_argument.hpp>
#include <ustl/fmt/details/basic_format_parse_context.hpp>

namespace ustl::fmt::details {
    template <typename CharType, typename... Args>
    class BasicFormatString
    {
        typedef CharType    Char;
        typedef CompileTimeBasicFormatContext<CharType>  Context;

    public:
        template <typename T>
        // requires std::convertible_to<T const &, views::BasicStringView<Char>> 
        USTL_FORCEINLINE USTL_CONSTEXPR
        BasicFormatString(T const &str)
            : str_(str)
        {
            typedef BasicFormatParseContext<Char>   ParseContext;
            // static_assert(traits::IsConvertibleV<T const &, views::BasicStringView<Char>>);

            // vformat_to(
            //     ParseContext(str_, sizeof...(Args)),
            //     Context{type_list().data(), handle_list().data(), sizeof...(Args)}
            // );
        }

        template <typename T>
        USTL_CONSTEXPR
        static auto check()
        {
            typedef traits::RemoveRefT<T>   UnCvRef;
            CompileTimeHandle<Char> handle;
            if (determine_arg_t<Context, UnCvRef>() == ArgType::Custom) {
                handle.template enable<UnCvRef>();
            }

            return handle;
        }

        // BasicFormatString(RuntimeFormatString<Char> s) USTL_NOEXCEPT
        //   : str_(s.str_) {}

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto get() const USTL_NOEXCEPT-> views::BasicStringView<Char> 
        { return str_; }
    
    // private:
    //     typedef collections::Array<ArgType, sizeof...(Args)>    TypeList;
    //     USTL_CONSTEXPR
    //     static auto type_list() -> TypeList
    //     {  return {}; }

    //     typedef collections::Array<CompileTimeHandle<Char>, sizeof...(Args)>    HandleList;
    //     USTL_CONSTEXPR
    //     static auto handle_list() -> HandleList
    //     {  return {}; }

    private:
        views::BasicStringView<Char> str_;
    };

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_BASIC_FORMAT_STRING_HPP