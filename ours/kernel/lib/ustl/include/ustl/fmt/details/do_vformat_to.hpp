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

#ifndef USTL_FMT_DETAILS_DO_VFORMAT_TO_HPP
#define USTL_FMT_DETAILS_DO_VFORMAT_TO_HPP 1

#include <ustl/config.hpp>

#include <ustl/iterator/traits.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/is_same.hpp>

#include <ustl/fmt/error.hpp>
#include <ustl/fmt/details/basic_format_context.hpp>
#include <ustl/fmt/details/basic_format_parse_context.hpp>
#include <ustl/fmt/details/basic_format_args.hpp>
#include <ustl/fmt/details/parse_arg_id.hpp>
#include <ustl/fmt/details/buffer_format.hpp>
#include <ustl/fmt/details/validate_argument.hpp>

// Support for primitive types.
#include <ustl/fmt/details/formatter_bool.hpp>
#include <ustl/fmt/details/formatter_integer.hpp>
#include <ustl/fmt/details/formatter_cstring.hpp>
#include <ustl/fmt/details/formatter_pointer.hpp>
#include <ustl/fmt/details/formatter_monostate.hpp>

namespace ustl::fmt {
namespace details {
    template <typename Handle, typename Char>
    struct FormatterHandle
    {
        template <typename ParseContext>
        auto parse(ParseContext &) const -> typename ParseContext::IterMut
        {  return {};  }

        template <typename FormatContext>
        auto format(Handle, FormatContext &) const -> typename FormatContext::IterMut
        {    }

        template <typename FormatContext>
        auto formatted_size(Handle, FormatContext &) const -> usize 
        {  return {};  }
    };


    // This function is not user facing, so it can directly use the non-standard
    // types of the "variant".
    template <class Char>
    USTL_CONSTEXPR
    auto compile_time_visit_format_arg(BasicFormatParseContext<Char> &parse_context,
                          CompileTimeBasicFormatContext<Char> &context,
                          ArgType const type) USTL_NOEXCEPT -> void
    {
        switch (type) {
            case ArgType::None:
                // std::throw_format_error("Invalid argument");
            case ArgType::Boolean:
                return compile_time_validate_argument<Char, bool>(parse_context, context);
            case ArgType::Char:
                return compile_time_validate_argument<Char, Char>(parse_context, context);
            case ArgType::I8:
                return compile_time_validate_argument<Char, i8>(parse_context, context);
            case ArgType::I16:
                return compile_time_validate_argument<Char, i16>(parse_context,context);
            case ArgType::I32:
                return compile_time_validate_argument<Char, i32>(parse_context,context);
            case ArgType::I64:
                return compile_time_validate_argument<Char, i64>(parse_context,context);
            // case ArgType::I128:
                // return validate_argument<Char, i128>(parse_context,context);
            case ArgType::U8:
                return compile_time_validate_argument<Char, u8>(parse_context,context);
            case ArgType::U16:
                return compile_time_validate_argument<Char, u16>(parse_context, context);
            case ArgType::U32:
                return compile_time_validate_argument<Char, u32>(parse_context,context);
            case ArgType::U64:
                return compile_time_validate_argument<Char, u64>(parse_context,context);
            // case ArgType::U128:
                // return validate_argument<Char, u128>(parse_context,context);
            case ArgType::F32:
                return compile_time_validate_argument<Char, f32, true>(parse_context, context);
            case ArgType::F64:
                return compile_time_validate_argument<Char, f64, true>(parse_context, context);
            case ArgType::CString:
                return compile_time_validate_argument<Char, Char const *, true>(parse_context, context);
            case ArgType::Pointer:
                return compile_time_validate_argument<Char, const void *>(parse_context, context);
            case ArgType::Custom:
            //   std::throw_format_error("Handle should use validate_handle_argument");
            default:
        }
        // std::throw_format_error("Invalid argument");
    }

    template <typename Context>
    struct Visitor
    {
        typedef BasicFormatArg<Context>::Handle     Handle;

        template <typename T>
        auto operator()(T &t) -> void
        {}

        auto operator()(Handle &t) -> void
        {}
    };

    template <typename Iterator, typename ParseContext, typename Context>
    USTL_CONSTEXPR
    auto try_replace_field(Iterator begin, Iterator end, ParseContext &parse_context, Context &context)
        -> Result<Iterator>
    {
        typedef typename BasicFormatArg<Context>::Handle        ContextHandle;
        typedef typename iter::IterTraits<Iterator>::Element    Char;

        ParseNumberResult r = parse_arg_id(begin, end, parse_context);

        if (r.last == end) {
            return ustl::err(FmtErr::ExpectedRightBraceButNot);
            // std::throw_format_error(
            //    "The argument index should end with a ':' or a '}'");
        }

        switch (*r.last) {
            case Char(':'): {
                // The arg-id has a format-specifier, advance the input to the format-spec.
                parse_context.advance_to(r.last + 1);
                break;
            }
            case Char('}'): {
                // The arg-id has no format-specifier.
                parse_context.advance_to(r.last);
                break;
            }
            default: {
                return ustl::err(FmtErr::ExpectedRightBraceButNot);
            }
        }

        bool const parse = (*r.last == Char(':'));
        USTL_IF_CONSTEXPR(traits::IsSameV<Context, CompileTimeBasicFormatContext<Char>>) {
            ArgType type = context.arg(r.value);
            if (type == ArgType::None) {
                return ustl::err(FmtErr::ArgsIndexOutOfRange);
            } else if (type == ArgType::Custom) {
                context.handle(r.value).parse(parse_context);
            } else if (parse) {
                compile_time_visit_format_arg(parse_context, context, type);
            }
        } else {
            // Runtime logic.
            auto const visitor = [&] (auto &arg) -> Result<void> {
                USTL_IF_CONSTEXPR(traits::IsSameV<decltype(arg), Monostate>) {
                    return ustl::err(FmtErr::InvalidArgIndex);
                    // std::throw_format_error("The argument index value is too large for "
                                        //   "the number of arguments supplied");
                } else USTL_IF_CONSTEXPR(traits::IsSameV<decltype(arg), ContextHandle>) {
                    // call user-custom parser.
                    arg.format(parse_context, context);
                } else {
                    Formatter<traits::RemoveRefT<decltype(arg)>, Char> formatter;
                    if (parse) {
                        parse_context.advance_to(formatter.parse(parse_context));
                    }
                    context.advance_to(formatter.format(arg, context));
                }

                return ustl::ok();
           };

           visit_format_arg(Visitor<Context>(), context.arg(r.value));
        }

        begin = parse_context.begin();
        if (begin == end || *begin != Char('}')) {
            return ustl::err(FmtErr::MissingRightBrace);
        }
        return ustl::ok(++begin);
    }

    /// Implementation of the common logic.
    template <typename ParseContext, typename Context>
    auto do_vformat_to(ParseContext &&parse_context, Context &&context)
        -> Result<typename Context::IterMut>
    {
        typedef typename Context::IterMut           IterMut;
        typedef typename ParseContext::Char         Char;
        static_assert(traits::IsSameV<Char, typename Context::Char>);

        auto to_return = context.out();
        auto begin = parse_context.begin(), end = parse_context.end();

        while (begin != end) {
            if (Char('{') == *begin) {
                ++begin;
                if (begin == end) {
                    // Though in libc++16 terminatation at left brace is seen as a exception,
                    // we should try to treat it as a choice stem from personality of user yet.
                    // therefore, just giving out a bit of doubt is ok.
                    return ustl::err(FmtErr::TerminatesAtLeftBrace);
                } else if (Char('{') != *begin) {
                    context.advance_to(ustl::move(to_return));
                    auto replace_result = try_replace_field(begin, end, parse_context, context);
                    if (!replace_result.has_value()) {
                        // return replace_result;
                    }

                    begin = replace_result.value();
                    to_return = context.out();

                    // The output is written and begin points to the next character. So
                    // start the next iteration.
                    continue;
                }
            } else if (Char('}') == *begin) {
                ++begin;
                if (begin == end || *begin != Char('}')) {
                    return ustl::err(FmtErr::InvalidEscapeSequence);
                }
            }

            // Regular case, copy the character to the output verbatim.
            *to_return++ = *begin++;
        }

        return ustl::ok(to_return);
    }

    template <typename Iterator, typename Char, typename FormatIterator>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto do_vformat_to(
        Iterator out,
        views::BasicStringView<Char> fmt,
        BasicFormatArgsRebind<FormatIterator, Char> args) USTL_NOEXCEPT
        -> Result<Iterator>
    {
        USTL_IF_CONSTEXPR(traits::IsSameV<Iterator, FormatIterator>) {
            return do_vformat_to(
                BasicFormatParseContext{fmt, args.size()},
                create_format_context(ustl::move(out), args)
            );
        }

        FormatBuffer<Iterator, Char> buffer{ustl::move(out)};
        do_vformat_to(
            BasicFormatParseContext{fmt, args.size()},
            create_format_context(buffer.make_output_iterator(), args)
        );

        return ustl::ok(ustl::move(buffer).out_it());
    }

} // namespace ustl::fmt::details

    template <typename Char>
    struct Formatter<FormatArg::Handle, Char>
        : public details::FormatterHandle<FormatArg::Handle, Char>
    {};

    template <typename Char>
    struct Formatter<WFormatArg::Handle, Char>
        : public details::FormatterHandle<WFormatArg::Handle, Char>
    {};
}

#endif // #ifndef USTL_FMT_DETAILS_DO_VFORMAT_TO_HPP