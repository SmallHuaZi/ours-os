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
#ifndef USTL_UTIL_TYPES_LIST_HPP
#define USTL_UTIL_TYPES_LIST_HPP 1

#include <ustl/config.hpp>
#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/conditional.hpp>
#include <ustl/util/type-algos.hpp>

namespace ustl {
    template <typename... Elems>
    struct TypeList {
        USTL_FORCEINLINE USTL_CONSTEXPR
        static auto size() -> usize {
            return sizeof...(Elems);
        }
    };

    /// TypeAlgos::DoAt<TypeList>
    template <typename T, typename... Ts>
    struct TypeAlgos::DoAt<TypeList<T, Ts...>, 0> {
        typedef T  Type;
    };

    template <usize I, typename... Ts>
    struct TypeAlgos::DoAt<TypeList<Ts...>, I> {
        static_assert(sizeof...(Ts) + 1 != 0, "[ustl-error] Unsupport empty types sequence.\n");
        typedef typename TypeAlgos::DoAt<TypeList<Ts...>, I - 1>::Type   Type;
    };

    /// TypeAlgos::DoFind<TypeList>
    template <typename ToFind, typename... Objects>
    struct TypeAlgos::DoFind<TypeList<Objects...>, ToFind> {
        template <usize I, typename TypeList>
        struct FindImpl;

        template <>
        struct FindImpl<0, TypeList<>> {
            typedef  NotFound   Type;
        };

        template <usize I, typename Head, typename... Tails>
        struct FindImpl<I, TypeList<Head, Tails...>> {
            typedef traits::ConditionalT<traits::IsSameV<ToFind, Head>, ToFind,
                                         typename FindImpl<I - 1, TypeList<Tails...>>::Type
                                         > Type;
        };

        typedef typename FindImpl<sizeof...(Objects), TypeList<Objects...>>::Type   Type;
    };

    /// TypeAlgos::DoPushFront<TypeList>
    template <typename ToPush, typename... Ts>
    struct TypeAlgos::DoPushFront<TypeList<Ts...>, ToPush> {
        typedef TypeList<ToPush, Ts...>     Type;
    };

    /// TypeAlgos::DoPushBack<TypeList>
    template <typename ToPush, typename... Ts>
    struct TypeAlgos::DoPushBack<TypeList<Ts...>, ToPush> {
        typedef TypeList<Ts..., ToPush>     Type;
    };

    /// TypeAlgos::DoPopFront<TypeList>
    template <typename Head, typename... Ts>
    struct TypeAlgos::DoPopFront<TypeList<Head, Ts...>> {
        typedef TypeList<Ts...>     Type;
    };

    /// TypeAlgos::DoPopBack<TypeList>
    template <typename... Ts>
    struct TypeAlgos::DoPopBack<TypeList<Ts...>> {
        template <typename TypeList, typename... Ts2>
        struct PopBack;

        template <typename TypeList, typename Back>
        struct PopBack<TypeList, Back> {
            typedef TypeList    Type;
        };

        template <typename TypeList, typename Head, typename... Ts2>
        struct PopBack<TypeList, Head, Ts2...>
            : public PopBack<TypeAlgos::PushBack<TypeList, Head>, Ts2...>
        {};

        typedef typename PopBack<TypeList<>, Ts...>::Type     Type;
    };

    /// `TypeAlogs::DoTruncate<TypeList>`
    template <usize Pos, typename... Ts>
    struct TypeAlgos::DoTruncate<TypeList<Ts...>, Pos> {
        static_assert(TypeList<Ts...>::size() > Pos, "[ustl-error] invalid range");

        template <typename TypeList, usize Count, typename... Ts2>
        struct Truncate;

        template <typename TypeList, typename... Ts2>
        struct Truncate<TypeList, 0, Ts2...> {
            typedef TypeList    Type;
        };

        template <typename TypeList, usize Count, typename Head, typename... Ts2>
        struct Truncate<TypeList, Count, Head, Ts2...>
            : public Truncate<TypeAlgos::PushBack<TypeList, Head>, Count - 1, Ts2...>
        {};

        typedef typename Truncate<TypeList<>, Pos, Ts...>::Type     Type;
    };

    /// `TypeAlogs::DoInvert<TypeList>`
    template <>
    struct TypeAlgos::DoInvert<TypeList<>> {
        typedef TypeList<>  Type;
    };

    template <typename T, typename... Ts>
    struct TypeAlgos::DoInvert<TypeList<T, Ts...>> {
        typedef typename TypeAlgos::Invert<TypeList<Ts...>> SubListInverted;
        typedef TypeAlgos::PushBack<SubListInverted, T>     Type;
    };

    /// TypeAlgos::DoRange<TypeList>
    template <usize Start, usize End, typename... Types>
    struct TypeAlgos::DoRange<TypeList<Types...>, Start, End> {
        typedef TypeAlgos::Truncate<TypeList<Types...>, End>  PrevHalf;
        typedef TypeAlgos::Invert<PrevHalf>   PrevHalfInverted;
        typedef TypeAlgos::Truncate<PrevHalfInverted, PrevHalfInverted::size() - Start>    Type;
    };

    /// TypeAlgos::DoSize<TypeList>
    template <>
    struct TypeAlgos::DoSize<TypeList<>>
        : public traits::IntConstant<0>
    {};

    template <typename T, typename... Ts>
    struct TypeAlgos::DoSize<TypeList<T, Ts...>>
        : public traits::IntConstant<sizeof(T) + TypeAlgos::SizeV<TypeList<Ts...>>>
    {};

    /// TypeAlgos::DoMerge<TypeList, Other>
    template <typename... Types1, typename... Types2>
    struct TypeAlgos::DoMerge<TypeList<Types1...>, TypeList<Types2...>> {
        typedef ustl::TypeList<Types1..., Types2...>    Type;
    };

    /// TypeListUnique<TypeList>
    template <typename... Types>
    struct TypeAlgos::DoUnique<TypeList<Types...>> {
        template <typename TypeList>
        struct Unique;

        template <>
        struct Unique<TypeList<>> {
            typedef TypeList<>  Type;
        };

        template <typename Head, typename... Tails>
        struct Unique<TypeList<Head, Tails...>> {
            typedef typename Unique<TypeList<Tails...>>::Type   UniqueTails;
            typedef ustl::traits::ConditionalT<
                ContainsV<UniqueTails, Head>,
                UniqueTails,
                PushFront<UniqueTails, Head>
            > Type;
        };

        typedef typename Unique<TypeList<Types...>>::Type   Type;
    };

    template <typename Object, typename... Types>
    struct TypeAlgos::DoGetPos<TypeList<Types...>, Object>{
        template <usize I, typename... Ts>
        struct GetPos;

        template <usize I, typename T, typename... Ts>
        struct GetPos<I, T, Ts...>
            : traits::IntConstant<traits::IsSameV<T, Object> ? I : GetPos<I + 1, Ts...>()>
        {};

        template <usize I>
        struct GetPos<I>
            : public traits::IntConstant<kNotFound>
        {};

        USTL_CXX11_CONSTEXPR
        static usize const VALUE = GetPos<0, Types...>();
    };

    /// TypeAlgos::DoTransform<TypeList>
    template <template <typename> typename F, typename... Types>
    struct TypeAlgos::DoTransform<TypeList<Types...>, F> {
        template <typename... Ts>
        struct Impl;

        template <>
        struct Impl<> {
            typedef ustl::TypeList<>    Type;
        };

        template <typename T, typename... Ts>
        struct Impl<T, Ts...> {
            typedef typename Impl<Ts...>::Type  NextIteration;
            typedef typename F<T>::Type     Object;
            typedef PushFront<NextIteration, Object>    Type;
        };

        typedef typename Impl<Types...>::Type   Type;
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_TYPES_LIST_HPP