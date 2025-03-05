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

#ifndef USTL_RESULT_HPP
#define USTL_RESULT_HPP 1

#include <ustl/details/storage.hpp>

#include <ustl/traits/cv.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_array.hpp>
#include <ustl/traits/invoke_result.hpp>

#include <ustl/function/invoke.hpp>
#include <ustl/util/move.hpp>

namespace ustl {
    template <typename T = void>
    struct Ok
    {
        traits::RemoveRefT<T> value_;
    };

    template <>
    struct Ok<void>
    {
        Monostate value_;
    };

    template <typename E = void>
    struct Err
    {
        traits::RemoveRefT<E> error_;
    };

    template <>
    struct Err<void>
    {};

    template <typename T, typename E = void, typename Policy = void>
    class Result
    {
        typedef Result     Self;
        typedef result::StorageTrivial<T, E>    Storage;

    public:
        typedef typename Storage::Error     Error;
        typedef typename Storage::Element   Element;
        typedef Element *           PtrMut;
        typedef Element const *     Ptr;
        typedef Element &           RefMut;
        typedef Element const &     Ref;

        USTL_FORCEINLINE USTL_CONSTEXPR
        Result(Ok<T> &&ok) USTL_NOEXCEPT
            : storage_(Inplace<Element>(), ustl::forward<Element>(ok.value_))
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR
        Result(Err<E> &&err) USTL_NOEXCEPT
            : storage_(Inplace<Error>(), ustl::forward<Error>(err.error_))
        {}

        auto is_ok() const -> bool
        {  return this->storage_.status_.has_value();  }

        auto is_err() const -> bool
        {  return !this->is_ok();  }

        /// Calls `f` if the `Result` is Ok, otherwise returns the Error value of self.
        /// This function can be used for control flow based on Result values.
        template <typename F>
        auto and_then(F &&f) && -> Result<traits::InvokeResultT<F, Element &&>, E>
        {
            typedef traits::InvokeResultT<F, Element &&>    U;
            if (*this) {
                return Result<U, E>(function::invoke(f, ustl::move(this->storage_.value_)));
            }
            return Err(ustl::move(this->storage_.error_));
        }

        template <typename F>
        auto and_then(F &&f) const & -> Result<traits::InvokeResultT<F, Element const &>, E>
        {
            typedef traits::InvokeResultT<F, Element const &>    U;
            if (*this) {
                return Result<U, E>(function::invoke(f, this->storage_.value_));
            }
            return Err(this->storage_.error_);
        }

        /// Returns res if the Result is Err, otherwise returns the Ok value of self.
        /// Arguments passed to or are eagerly evaluated; if you are passing the result 
        /// of a function call, 
        /// it is recommended to use or_else, which is lazily evaluated.
        template <typename F>
        auto or_else(F &&f) && -> Result<traits::InvokeResultT<F, Element &&>, E>
        {
            typedef traits::InvokeResultT<F, Element &&>    U;
            static_assert(traits::IsSameV<T, U>, "");
            if (*this) {
                return *this;
            }
            return Result<U, E>(function::invoke(f, ustl::move(this->storage_.value_)));
        }

        template <typename F>
        auto or_else(F &&f) const & -> Result<traits::InvokeResultT<F, Element const &>, E>
        {
            typedef traits::InvokeResultT<F, Element &&>    U;
            static_assert(traits::IsSameV<T, U>, "");
            if (*this) {
                return *this;
            }
            return Result<U, E>(function::invoke(f, this->storage_.value_));
        }

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto inspect(F &&f) && -> Result<traits::InvokeResultT<F, Element &&>, E>
        {}

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto inspect(F &&f) && -> Result<traits::InvokeResultT<F, Element const &>, E>
        {}

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto inspect(F &&f) const & -> Result<traits::InvokeResultT<F, Element const &>, E>
        {}

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto inspect_err(F &&f) && -> Result<traits::InvokeResultT<F, Element const &>, E>
        {}

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto inspect_err(F &&f) const & -> Result<traits::InvokeResultT<F, Element const &>, E>
        {}
        
        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto map(F &&f) && -> Result<traits::InvokeResultT<F, Element &&>, E>
        {
            typedef traits::RemoveCvT<traits::InvokeResultT<F, Element &&>>    U;
            static_assert(!traits::IsArrayV<U>, "Result of f(ustl::move(value())) should not be an Array");
            static_assert(!traits::IsSameV<U, std::in_place_t>, "Result of f(ustl::move(value())) should not be std::in_place_t");
            // static_assert(!traits::IsSameV<U, std::nullopt_t>, "Result of f(ustl::move(value())) should not be std::nullopt_t");
            static_assert(std::is_object_v<U>, "Result of f(ustl::move(value())) should be an object type");
            if (*this) {
                return Result<U, E>(function::invoke(f, this->storage_.value_));
            }
            return Result<U, E>();
        }

        template <typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto map(F &&f) const && -> Result<traits::InvokeResultT<F, Element const &>, E>
        {
            typedef traits::RemoveCvT<traits::InvokeResultT<F, Element const &&>>    U;
            static_assert(!traits::IsArrayV<U>, "Result of f(ustl::move(value())) should not be an Array");
            static_assert(!traits::IsSameV<U, std::in_place_t>, "Result of f(ustl::move(value())) should not be std::in_place_t");
            // static_assert(!traits::IsSameV<U, std::nullopt_t>, "Result of f(ustl::move(value())) should not be std::nullopt_t");
            static_assert(std::is_object_v<U>, "Result of f(ustl::move(value())) should be an object type");
            if (*this) {
                return Result<U, E>(function::invoke(f, this->storage_.value_));
            }
            return Result<U, E>();
        }

        template <typename U, typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto map_or(U &&u, F &&f) &&
            -> Result<traits::InvokeResultT<F, Element &&>, E>
        {}

        template <typename U, typename F>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto map_or(U &&u, F &&f) const &
            -> Result<traits::InvokeResultT<F, Element const &>, E>
        {}

        template <typename F1, typename F2>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto map_or_else(F1 &&f1, F2 &&f2) const &
            -> Result<traits::InvokeResultT<F1, Element const &>, E>
        {
            typedef traits::InvokeResultT<F1, Element const &>   R1;
            typedef traits::InvokeResultT<F2, Element const &>   R2;
            static_assert(traits::IsSameV<R1, R2>, "Result of F1 and F2 should be identical");
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto unwrap() -> RefMut
        {  return this->storage_.value_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto unwrap() const -> Ref
        {  return this->storage_.value_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto error() -> Error
        {  return this->storage_.error_;  }

        auto operator&(Self &&other) -> Self &
        {}

        auto operator|(Self &&other) -> Self &
        {}

        USTL_FORCEINLINE
        operator bool() const USTL_NOEXCEPT
        {  return this->is_ok();  }
    
    private:
        result::StorageTrivial<T, E> storage_;
    };

    USTL_FORCEINLINE USTL_CONSTEXPR
    auto ok() -> Ok<>
    {  return Ok<>();  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto ok(T t) -> Ok<T>
    {  return Ok<T>(ustl::forward<T>(t));  }

    template <typename E>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto err(E e) -> Err<E>
    {  return Err<E>(ustl::forward<E>(e));  }

} // namespace ustl 

#endif // #ifndef USTL_RESULT_HPP