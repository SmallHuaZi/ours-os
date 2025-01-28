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

#ifndef USTL_FMT_DETAILS_VALIDATE_ARGUMENT_HPP
#define USTL_FMT_DETAILS_VALIDATE_ARGUMENT_HPP 1

#include <ustl/fmt/details/fwd.hpp>
#include <ustl/fmt/details/basic_format_arg.hpp>

namespace ustl::fmt::details {

    template <typename CharType>
    struct CompileTimeHandle
    {
        typedef CharType    Char;
        typedef BasicFormatParseContext<Char>   ParseContext;

        template <typename OtherParseContext>
        USTL_CONSTEXPR
        auto parse(OtherParseContext &ctx) const -> void
        {
            parse_(ctx);
        }

        template <typename T>
        USTL_CONSTEXPR
        auto enable() -> void
        {
            parse_ = [] (ParseContext &ctx) {
                Formatter<T, Char> f;
                ctx.advance_to(f.parse(ctx));
            };
        }

        // Before calling parse the proper handler needs to be set with enable.
        // The default handler isn't a core constant expression.
        USTL_CONSTEXPR
        CompileTimeHandle()
            : parse_([] (ParseContext &) {})
        {}

    private:
        auto (*parse_)(ParseContext &) -> void;
    };

    // Dummy format_context only providing the parts used during constant
    // validation of the basic_format_string.
    template <typename CharType>
    struct CompileTimeBasicFormatContext
    {
        typedef CharType    Char;

        USTL_CONSTEXPR
        explicit CompileTimeBasicFormatContext(
            ArgType const *args, 
            CompileTimeHandle<Char> const *handles, 
            usize size)
            : args_(args), handles_(handles), size_(size)
        {}

        // During the compile-time validation nothing needs to be written.
        // Therefore all operations of this iterator are a NOP.
        struct Iter {
            constexpr Iter &operator=(Char)
            { return *this; }

            constexpr Iter &operator*()
            { return *this; }

            constexpr Iter operator++(int)
            { return *this; }
        };

        USTL_CONSTEXPR
        auto arg(usize id) const -> ArgType {
            if (id >= size_) {
                // std::throw_format_error("The argument index value is too large for the "
                                    // "number of arguments supplied");
            }
            return args_[id];
        }

        USTL_CONSTEXPR
        auto handle(usize id) const -> const CompileTimeHandle<Char> & {
            if (id >= size_) {
                // std::throw_format_error("The argument index value is too large for the "
                                    // "number of arguments supplied");
            }
            return handles_[id];
        }

        USTL_CONSTEXPR
        auto out() -> Iter
        { return {}; }

        USTL_CONSTEXPR
        auto advance_to(Iter) -> void
        {}

    private:
        usize size_;
        ArgType const *args_;
        CompileTimeHandle<Char> const *handles_;
    };

    template <typename Char, class T, bool HasPrecision = false>
    USTL_CONSTEXPR 
    auto compile_time_validate_argument(BasicFormatParseContext<Char> &parse_context,
                           CompileTimeBasicFormatContext<Char> &context) -> void 
    {
        auto validate_type = [] (ArgType type) {
            // LWG3720 originally allowed "signed or unsigned integer types", however
            // the final version explicitly changed it to "*standard* signed or unsigned
            // integer types". It's trivial to use 128-bit integrals in libc++'s
            // implementation, but other implementations may not implement it.
            // (Using a width or precision, that does not fit in 64-bits, sounds very
            // unlikely in real world code.)
            switch (type) {
                case ArgType::I8:
                case ArgType::U8:
                case ArgType::I16:
                case ArgType::U16:
                    return;

                default: // Error
            }
        };

        Formatter<T, Char> formatter;
        parse_context.advance_to(formatter.parse(parse_context));
        if (formatter.parser_.width_as_arg_) {
            validate_type(context.arg(formatter.parser_.width_));

        }

        USTL_IF_CONSTEXPR(HasPrecision) {
            if (formatter.parser_.precision_as_arg_) {
                validate_type(context.arg(formatter.parser_.precision_));
            }
        }
    }
}

#endif // #ifndef USTL_FMT_DETAILS_VALIDATE_ARGUMENT_HPP