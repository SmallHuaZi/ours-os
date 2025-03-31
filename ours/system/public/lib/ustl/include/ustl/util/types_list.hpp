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

namespace ustl {
    template <typename... Elems>
    struct TypeList
    {
        USTL_FORCEINLINE USTL_CONSTEXPR 
        static auto size() -> usize
        {  return sizeof...(Elems);  }
    };

    /// TypeListAt<This, Idx>
    template <typename This, usize I>
    struct TypeListAt;

    template <typename T, typename... Ts>
    struct TypeListAt<TypeList<T, Ts...>, 0>
    {  typedef T   Type;  }; 

    template <usize I, typename T, typename... Ts>
    struct TypeListAt<TypeList<T, Ts...>, I>
    { 
        static_assert(sizeof...(Ts) + 1 != 0, "[ustl-error] Unsupport empty types sequence.\n"); 
        typedef typename TypeListAt<TypeList<Ts...>, I - 1>::Type   Type;
    };

    template <typename This ,usize I>
    using TypeListAtT = typename TypeListAt<This, I>::Type;

    /// `TypeListPushFront<This, ToPush>`
    template <typename This, typename ToPush>
    struct TypeListPushFront;

    template <typename ToPush, typename... Ts>
    struct TypeListPushFront<TypeList<Ts...>, ToPush>
    {  typedef TypeList<ToPush, Ts...>     Type;  };

    template <typename This ,typename ToPush>
    using TypeListPushFrontT = typename TypeListPushFront<This, ToPush>::Type;

    /// `TypeListPopFront<This>`
    template <typename This>
    struct TypeListPopFront;

    template <typename Head, typename... Ts>
    struct TypeListPopFront<TypeList<Head, Ts...>>
    {  typedef TypeList<Ts...>     Type;  };

    template <typename This>
    using TypeListPopFrontT = typename TypeListPopFront<This>::Type;

    /// `TypeListPushBack<This, ToPush>`
    template <typename This, typename ToPush>
    struct TypeListPushBack;

    template <typename ToPush, typename... Ts>
    struct TypeListPushBack<TypeList<Ts...>, ToPush>
    {  typedef TypeList<Ts..., ToPush>     Type;  };

    template <typename This ,typename ToPush>
    using TypeListPushBackT = typename TypeListPushBack<This, ToPush>::Type;

    /// `TypeListPopBack<This>`
    template <typename This>
    struct TypeListPopBack;

    template <typename... Ts>
    struct TypeListPopBack<TypeList<Ts...>>
    {
        template <typename TypeList, typename... Ts2>
        struct DoPopBack;

        template <typename TypeList, typename Back>
        struct DoPopBack<TypeList, Back>
        {  typedef TypeList    Type;  };

        template <typename TypeList, typename Head, typename... Ts2>
        struct DoPopBack<TypeList, Head, Ts2...>
            : public DoPopBack<TypeListPushBackT<TypeList, Head>, Ts2...>
        {};

        typedef typename DoPopBack<TypeList<>, Ts...>::Type     Type;
    };

    template <typename This>
    using TypeListPopBackT = typename TypeListPopBack<This>::Type;

    /// `TypeListTruncate<This, Pos>`
    template <typename This, usize Pos>
    struct TypeListTruncate;

    template <usize Pos, typename... Ts>
    struct TypeListTruncate<TypeList<Ts...>, Pos>
    {
        static_assert(TypeList<Ts...>::size() > Pos, "[ustl-error] invalid range");

        template <typename TypeList, usize Count, typename... Ts2>
        struct DoTruncate;

        template <typename TypeList, typename... Ts2>
        struct DoTruncate<TypeList, 0, Ts2...>
        {  typedef TypeList    Type;  };

        template <typename TypeList, usize Count, typename Head, typename... Ts2>
        struct DoTruncate<TypeList, Count, Head, Ts2...>
            : public DoTruncate<TypeListPushBackT<TypeList, Head>, Count - 1, Ts2...>
        {};

        typedef typename DoTruncate<TypeList<>, Pos, Ts...>::Type     Type;
    };

    template <typename This, usize Pos>
    using TypeListTruncateT = typename TypeListTruncate<This, Pos>::Type;

    /// TypeListInvert<This>
    template <typename This>
    struct TypeListInvert;

    template <typename T>
    struct TypeListInvert<TypeList<T>>
    {  typedef TypeList<T>    Type;  };

    template <typename T, typename... Ts>
    struct TypeListInvert<TypeList<T, Ts...>>
    {
        typedef typename TypeListInvert<TypeList<Ts...>>::Type SubTypeListInverted;
        typedef typename TypeListPushBack<SubTypeListInverted, T>::Type   Type;
    };

    template <typename This>
    using TypeListInvertT = typename TypeListInvert<This>::Type;

    /// `TypeListSubList<This, Start, End>`: get the slice range from `|Start|` to `|End|` of `|This|`
    template <typename This, usize Start, usize End>
    struct TypeListSubList;

    template <usize Start, usize End, typename... Types>
    struct TypeListSubList<TypeList<Types...>, Start, End>
    {
        typedef TypeListTruncateT<TypeList<Types...>, End>  PrevHalf;
        typedef TypeListInvertT<PrevHalf>   PrevHalfInverted;
        typedef TypeListTruncateT<PrevHalfInverted, PrevHalfInverted::size() - Start>    Type;
    };

    /// TypeListSizeOf
    template <typename This>
    struct TypeListSizeOf;

    template <>
    struct TypeListSizeOf<TypeList<>>
        : public traits::IntConstant<0>
    {};

    template <typename T, typename... Ts>
    struct TypeListSizeOf<TypeList<T, Ts...>>
        : public traits::IntConstant<sizeof(T) + TypeListSizeOf<TypeList<Ts...>>()>
    {};

    template <typename TypeList>
    USTL_CONSTEXPR
    static auto const TypeListSizeOfV = TypeListSizeOf<TypeList>::VALUE;

} // namespace ustl

#endif // #ifndef USTL_UTIL_TYPES_LIST_HPP