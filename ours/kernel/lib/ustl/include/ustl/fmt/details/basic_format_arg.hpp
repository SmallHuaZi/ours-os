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

#ifndef USTL_FMT_DETAILS_BASIC_FORMAT_ARG_HPP
#define USTL_FMT_DETAILS_BASIC_FORMAT_ARG_HPP 1

#include <ustl/placeholders.hpp>
#include <ustl/util/move.hpp>
#include <ustl/util/types_list.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/fmt/details/fwd.hpp>

namespace ustl::fmt::details {
    enum class ArgType {
        None,
        I8,
        I16,
        I32,
        I64,
        I128,
        ISize,
        U8,
        U16,
        U32,
        U64,
        U128,
        USize,
        Char,
        Boolean,
        F32,
        F64,
        Pointer,
        CString,
        String,
        Custom,
    };

    template <typename Context>
    struct BasicFormatArgPayload
    {
        typedef typename Context::Char   Char;

        explicit BasicFormatArgPayload(USTL_MAYBE_UNUSED Monostate) USTL_NOEXCEPT
            : type_(ArgType::None)
        {}

        explicit BasicFormatArgPayload(i8 v) USTL_NOEXCEPT
            : type_(ArgType::I8), i8_(v)
        {}

        explicit BasicFormatArgPayload(i16 v) USTL_NOEXCEPT
            : type_(ArgType::I16), i16_(v)
        {}

        explicit BasicFormatArgPayload(i32 v) USTL_NOEXCEPT
            : type_(ArgType::I32), i32_(v)
        {}

        explicit BasicFormatArgPayload(i64 v) USTL_NOEXCEPT
            : type_(ArgType::I64), i64_(v)
        {}

        explicit BasicFormatArgPayload(u8 v) USTL_NOEXCEPT
            : type_(ArgType::I16), u8_(v)
        {}

        explicit BasicFormatArgPayload(u16 v) USTL_NOEXCEPT
            : type_(ArgType::U16), u16_(v)
        {}

        explicit BasicFormatArgPayload(u32 v) USTL_NOEXCEPT
            : type_(ArgType::U32), u32_(v)
        {}

        explicit BasicFormatArgPayload(u64 v) USTL_NOEXCEPT
            : type_(ArgType::U64), u64_(v)
        {}

        explicit BasicFormatArgPayload(bool v) USTL_NOEXCEPT
            : type_(ArgType::Boolean), boolean_(v)
        {}

        explicit BasicFormatArgPayload(Char v) USTL_NOEXCEPT
            : type_(ArgType::Char), char_(v)
        {}

        explicit BasicFormatArgPayload(char const *v) USTL_NOEXCEPT
            : type_(ArgType::CString), cstring_(v)
        {}

        explicit BasicFormatArgPayload(void *v) USTL_NOEXCEPT
            : type_(ArgType::Pointer), pointer_(v)
        {}

        template <typename T>
        explicit BasicFormatArgPayload(T *v) USTL_NOEXCEPT
            : type_(ArgType::Custom), handle_(v)
        {}

        explicit operator bool() const USTL_NOEXCEPT
        {  return type_ != ArgType::None;  }

        struct Handle;
        union {
            Monostate monostate_;
            bool    boolean_;
            u8      u8_;
            u16     u16_;
            u32     u32_;
            u64     u64_;
            usize   usize_;
            i8      i8_;
            i16     i16_;
            i32     i32_;
            i64     i64_;
            isize   isize_;
            Char    char_;
            Handle  handle_;
            char const *cstring_;
            void *pointer_;
        };
        ArgType type_;
    };

    template <typename Context>
    struct BasicFormatArgPayload<Context>::Handle
    {
        const void *custom_;
    };

    template <typename Context>
    class BasicFormatArg
    {
        typedef BasicFormatArg                  Self;
        typedef BasicFormatArgPayload<Context>  Payload;

    public:
        typedef typename Payload::Handle        Handle;

        template <typename T = Monostate>
        USTL_FORCEINLINE USTL_CONSTEXPR
        BasicFormatArg(T v = T()) USTL_NOEXCEPT
            : payload_(v)
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto type() -> ArgType
        {  return payload_.type_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto data() -> Payload &
        {  return payload_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        explicit operator bool() USTL_NOEXCEPT
        {  return payload_.type_ != ArgType::None;  }

    private:
        Payload payload_;
    };


    template <typename Context, typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto create_format_arg(T &&value) USTL_NOEXCEPT -> BasicFormatArg<Context>
    {
        return BasicFormatArg<Context>(value);
    }

    template <typename Visitor, typename Context>
    decltype(auto) do_visit_format_arg(Visitor &&vis, BasicFormatArg<Context> arg)
    {
        typedef typename BasicFormatArg<Context>::Handle     Handle;
        auto payload = arg.data();
        switch (payload.type_) {
            case ArgType::None:
                return function::invoke(ustl::forward<Visitor>(vis), payload.monostate_);
            case ArgType::Boolean:
                return function::invoke(ustl::forward<Visitor>(vis), payload.boolean_);
            case ArgType::I8:
                return function::invoke(ustl::forward<Visitor>(vis), payload.i8_);
            case ArgType::I16:
                return function::invoke(ustl::forward<Visitor>(vis), payload.i16_);
            case ArgType::I32:
                return function::invoke(ustl::forward<Visitor>(vis), payload.i32_);
            case ArgType::I64:
                return function::invoke(ustl::forward<Visitor>(vis), payload.i64_);
            case ArgType::U8:
                return function::invoke(ustl::forward<Visitor>(vis), payload.u8_);
            case ArgType::U16:
                return function::invoke(ustl::forward<Visitor>(vis), payload.u16_);
            case ArgType::U32:
                return function::invoke(ustl::forward<Visitor>(vis), payload.u32_);
            case ArgType::U64:
                return function::invoke(ustl::forward<Visitor>(vis), payload.u64_);
        #ifdef USTL_FLOAT
            case ArgType::F32:
                return function::invoke(ustl::forward<Visitor>(vis), payload.f32_);
            case ArgType::F64:
                return function::invoke(ustl::forward<Visitor>(vis), payload.f64_);
        #endif
            case ArgType::CString:
                return function::invoke(ustl::forward<Visitor>(vis), payload.cstring_);
            case ArgType::Pointer:
                return function::invoke(ustl::forward<Visitor>(vis), payload.pointer_);
            case ArgType::Custom:
                return function::invoke(ustl::forward<Visitor>(vis), payload.handle_);
            default:
        }
    }

    template <typename Visitor, typename Context>
    decltype(auto) visit_format_arg(Visitor &&vis, BasicFormatArg<Context> arg)
    {
        typedef typename BasicFormatArg<Context>::Handle    Handle;

        auto payload = arg.data();
        switch (payload.type_) {
            case ArgType::I128: {
                // Handle handle{payload.i128_};
                // return function::invoke(forward<Visitor>(vis), handle);
            }
            case ArgType::U128: {
                // Handle handle{payload.u128_};
                // return function::invoke(forward<Visitor>(vis), handle);
            }
            default: {
                return do_visit_format_arg(ustl::forward<Visitor>(vis), arg);
            }
        }
    }

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_BASIC_FORMAT_ARG_HPP