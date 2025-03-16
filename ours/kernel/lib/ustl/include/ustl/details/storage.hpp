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
#ifndef USTL_RESULT_DETAILS_STORAGE_HPP
#define USTL_RESULT_DETAILS_STORAGE_HPP 1

#include <ustl/config.hpp>
#include <ustl/placeholders.hpp>
#include <ustl/initializer_list.hpp>
#include <ustl/traits/void.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl::result {
    enum class Status: u16 {
         // WARNING: These bits are not tracked by abi-dumper, but changing them will break ABI!
        None = 0,

        HaveValue = (1U << 0U),
        HaveError = (1U << 1U),

        // failed to complete a strong swap
        HaveLostConsistency = (1U << 3U),
        HaveValueLostConsistency = (1U << 0U) | (1U << 3U),
        HaveErrorLostConsistency = (1U << 1U) | (1U << 3U),
        HaveExceptionLostConsistency = (2U << 1U) | (1U << 3U),
        HaveErrorExceptionLostConsistency = (3U << 1U) | (1U << 3U),

        // can errno be set from this error?
        HaveErrorIsErrno = (1U << 4U),
        HaveErrorErrorIsErrno = (1U << 1U) | (1U << 4U),
        HaveErrorExceptionErrorIsErrno = (3U << 1U) | (1U << 4U),

        HaveErrorLostConsistencyErrorIsErrno = (1U << 1U) | (1U << 3U) | (1U << 4U),
        HaveErrorExceptionLostConsistencyErrorIsErrno = (3U << 1U) | (1U << 3U) | (1U << 4U),

        // value has been moved from
        HaveMovedFrom = (1U << 5U)       
    };

    struct StatusField
    {
        USTL_CONSTEXPR 
        StatusField()
            : status_(Status::None)
        {}

        USTL_CONSTEXPR 
        StatusField(Status v) USTL_NOEXCEPT
            : status_(v)
        {}

        USTL_CONSTEXPR 
        StatusField(Status v, u16 s) USTL_NOEXCEPT
            : status_(v)
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto has_value() const -> bool
        {  return (static_cast<u16>(status_) & static_cast<u16>(Status::HaveValue)) != 0;  }

        Status status_;
    };

    template <typename T, typename E>
    struct StorageTrivial
    {
        typedef StorageTrivial      Self;

        // Disable in place construction if they are the same type
        struct DisableInPlaceError {};
        struct DisableInPlaceElement {};

        typedef traits::ConditionalT<traits::IsSameV<T, E>, DisableInPlaceError, E>       VoidOrError;
        typedef traits::ConditionalT<traits::IsSameV<T, E>, DisableInPlaceElement, T>     VoidOrElement;
    
        typedef traits::DevoidT<VoidOrError>        Error;
        static_assert(!traits::IsVoidV<Error>, "Never be void");

        typedef traits::DevoidT<VoidOrElement>      Element;
        static_assert(!traits::IsVoidV<Element>, "Never be void");

        USTL_CONSTEXPR
        StorageTrivial() USTL_NOEXCEPT
            : empty_()
        {}

        StorageTrivial(Self &&) = default;
        StorageTrivial(Self const &) = default;
        auto operator=(Self &&) -> StorageTrivial & = default;
        auto operator=(Self const &) -> StorageTrivial & = default;
        ~StorageTrivial()
        {}

        USTL_CONSTEXPR
        explicit StorageTrivial(StatusField status) USTL_NOEXCEPT
            : empty_()
            , status_(status)
        {}

        template <typename... Args>
        USTL_CONSTEXPR
        explicit StorageTrivial(USTL_MAYBE_UNUSED Inplace<Element>, Args &&...args) 
            : value_(static_cast<Args &&>(args)...), 
              status_(Status::HaveValue)
        {}

        template <typename U, typename... Args>
        USTL_CONSTEXPR
        StorageTrivial(USTL_MAYBE_UNUSED Inplace<Element>, InitializerList<U> il, Args &&...args)
            : value_(il, static_cast<Args &&>(args)...), 
              status_(Status::HaveValue)
        {}

        template <typename... Args>
        USTL_CONSTEXPR
        explicit StorageTrivial(Inplace<Error>, Args &&...args)
            : error_(static_cast<Args &&>(args)...), 
              status_(Status::HaveError)
        {}

        template <typename U, typename... Args>
        USTL_CONSTEXPR
        StorageTrivial(Inplace<Error>, InitializerList<U> il, Args &&...args) 
            : error_(il, static_cast<Args &&>(args)...), 
              status_(Status::HaveError)
        {}

        USTL_CONSTEXPR 
        auto swap(StorageTrivial &other) USTL_NOEXCEPT -> void
        {
            // storage is trivial, so just use assignment
            auto temp = static_cast<StorageTrivial &&>(*this);
            *this = static_cast<StorageTrivial &&>(other);
            other = static_cast<StorageTrivial &&>(temp);
        }

        union {
            Monostate   empty_;
            Element     value_;
            Error       error_;
        };
        StatusField status_;
    };
}

#endif // #ifndef USTL_DETAILS_STORAGE_HPP