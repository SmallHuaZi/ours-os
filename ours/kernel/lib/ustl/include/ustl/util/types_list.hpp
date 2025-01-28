#ifndef USTL_UTIL_TYPES_LIST_H
#define USTL_UTIL_TYPES_LIST_H 1

#include <ustl/config.hpp>

namespace ustl {

    template <typename... Elems>
    struct TypesList
    {
        typedef TypesList    type;

        USTL_FORCEINLINE USTL_CONSTEXPR 
        static auto size() -> usize
        {  return sizeof...(Elems);  }
    };

    template <usize I, typename T>
    struct TypesListValueType;

    template <typename T, typename... Ts>
    struct TypesListValueType<0, TypesList<T, Ts...>>
    { 
        typedef T       Type; 
    }; 

    template <usize I, typename T, typename... Ts>
    struct TypesListValueType<I, TypesList<T, Ts...>>
    { 
        static_assert(sizeof...(Ts) + 1 != 0, "[ustl-error] Unsupport empty types sequence.\n"); 
        typedef typename TypesListValueType<I - 1, TypesList<Ts...>>::Type   
            Type;
    };

    template <usize I, typename T>
    using TypesListValueTypeT = typename TypesListValueType<I, T>::Type;

    /// `PushToTypeList<Front, ToPush, List>`
    ///
    /// # Example:
    ///     struct X {};
    ///     typedef TypeList<int, char, bool> MyList;
    ///     typedef PushToTypesList<true, X, MyList>  MyList2;
    ///     static_assert(ustl::traits::IsSameV<TypesListValueType<0, MyList2>, X>);
    ///     
    template <bool Front, typename ToPush, typename List>
    struct PushToTypesList;

    template <typename T, typename... Ts>
    struct PushToTypesList<true, T, TypesList<Ts...>>
    {
        typedef TypesList<T, Ts...>     Type;
    };

    template <typename T, typename... Ts>
    struct PushToTypesList<false, T, TypesList<Ts...>>
    {
        typedef TypesList<Ts..., T>     Type;
    };


    template <typename T>
    struct InvertTypesList;

    template <typename T>
    struct InvertTypesList<TypesList<T>>
    {
        typedef TypesList<T>    Type;
    };

    template <typename T, typename... Ts>
    struct InvertTypesList<TypesList<T, Ts...>>
    {
        typedef typename InvertTypesList<TypesList<Ts...>>::Type InvertedTypesList;
        typedef typename PushToTypesList<false, T, InvertedTypesList>::Type   Type;
    };

    template <typename T>
    using InvertTypesListT = typename InvertTypesList<T>::Type;

    template <usize Sum, typename T>
    struct SizeOfTypesListImpl;

    template <usize Sum, typename T>
    struct SizeOfTypesListImpl<Sum, TypesList<T>>
    {
        USTL_CONSTEXPR
        static usize const VALUE = Sum + sizeof(T);
    };

    template <usize Sum, typename T, typename... Ts>
    struct SizeOfTypesListImpl<Sum, TypesList<T, Ts...>>
    {
        USTL_CONSTEXPR static usize const VALUE = {
            Sum + SizeOfTypesListImpl<Sum + sizeof(T), TypesList<Ts...>>::VALUE
        };
    };

    template <typename TypesList>
    USTL_CONSTEXPR
    static auto const SIZE_OF_TYPES_LIST_V = SizeOfTypesListImpl<0, TypesList>::VALUE;

    static_assert(SIZE_OF_TYPES_LIST_V<TypesList<int, char>> == sizeof(int) + sizeof(char));

} // namespace ustl


#endif // #ifdef USTL_UTIL_TYPES_LIST_H