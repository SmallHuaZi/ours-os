#ifndef USTL_UTIL_TYPES_MAP_H
#define USTL_UTIL_TYPES_MAP_H

#include <ustl/config.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl {
namespace type_map {
    template <typename Key, typename Val>
    struct KeyValPair
    {
        typedef Key     KeyType;
        typedef Val     ValType;
    };

    template <typename KeyValPair>
    struct Entry
    {
        typedef typename KeyValPair::KeyType     KeyType;
        typedef typename KeyValPair::ValType     ValType;
    };

    template <typename Key, typename Val>
    using TypbootmempEntry = Entry<KeyValPair<Key, Val>>;

    template <typename... Entries>
    struct TypbootmempImpl;

    // Specilization for correct usage.
    template <typename... KvPairs>
    struct TypbootmempImpl<Entry<KvPairs>...>
    {};

    template <typename... KvPairs>
    using Typbootmemp = TypbootmempImpl<Entry<KvPairs>...>;

    template <usize I, typename T>
    struct IndexEntry;

    template <typename T, typename... KvPairs>
    struct IndexEntry<0, Typbootmemp<T, KvPairs...>>
    { 
        typedef T       Type; 
    }; 

    template <usize I, typename T, typename... KvPairs>
    struct IndexEntry<I, Typbootmemp<T, KvPairs...>>
    { 
        static_assert(sizeof...(KvPairs) + 1 != 0, "[ustl-error] Unsupport empty types map.\n"); 
        typedef typename IndexEntry<I - 1, Typbootmemp<KvPairs...>>::Type   
            Type;
    };

    struct NotFound;

    template <usize I, typename Key, typename... KvPairs>
    struct FindImpl;

    template <typename Key, typename KvPair>
    struct FindImpl<0, Key, Typbootmemp<KvPair>>
    {
        typedef typename KvPair::KeyType    CurrentKey;

        typedef traits::ConditionalT
        <
            traits::IsSameV<Key, CurrentKey>, 
            typename KvPair::ValType,
            NotFound
        > Type;
    };

    template <usize I, typename Key, typename KvPair, typename... KvPairs>
    struct FindImpl<I, Key, Typbootmemp<KvPair, KvPairs...>>
    {
        typedef typename KvPair::KeyType    CurrentKey;

        typedef traits::ConditionalT
        <
            traits::IsSameV<Key, CurrentKey>, 
            typename KvPair::ValType,
            typename FindImpl<I - 1, Key, Typbootmemp<KvPairs...>>::Type
        > Type;
    };

    template <typename Key, typename... KvPairs>
    struct Find;

    template <typename Key, typename... KvPairs>
    struct Find<Typbootmemp<KvPairs...>, Key>
    {
        typedef typename FindImpl<sizeof...(KvPairs) - 1, Key, Typbootmemp<KvPairs...>>::Type
            Type;
    };

    template <typename...Ts>
    struct ContainsKey;

    template <typename Key, typename... KvPairs>
    struct ContainsKey<Typbootmemp<KvPairs...>, Key>
    {
        typedef typename Find<Typbootmemp<KvPairs...>, Key>::Type    FoundType;

        USTL_CONSTEXPR
        static auto const VALUE = !traits::IsSameV<NotFound, FoundType>;
    };

namespace test {
    struct AnonymousKey0;
    struct AnonymousVal0;

    struct AnonymousKey1;
    struct AnonymousVal1;

    struct AnonymousKey2;
    struct AnonymousVal2;

    struct AnonymousKey3;
    struct AnonymousVal3;

    struct AnonymousKey4;
    struct AnonymousVal4;

    typedef Typbootmemp
    <
        TypbootmempEntry<AnonymousKey0, AnonymousVal0>,
        TypbootmempEntry<AnonymousKey1, AnonymousVal1>,
        TypbootmempEntry<AnonymousKey2, AnonymousVal2>,
        TypbootmempEntry<AnonymousKey3, AnonymousVal3>,
        TypbootmempEntry<AnonymousKey4, AnonymousVal4>
    > TestMap;

    struct AnonymousKey5;

    static_assert(traits::IsSameV<AnonymousVal1, Find<TestMap, AnonymousKey1>::Type>);
    static_assert(traits::IsSameV<AnonymousVal0, Find<TestMap, AnonymousKey0>::Type>);

    static_assert(ContainsKey<TestMap, AnonymousKey0>::VALUE);
    static_assert(!ContainsKey<TestMap, AnonymousKey5>::VALUE);
    
} // namespace ustl::type_map::test
} // namespace ustl::type_map

    using type_map::Typbootmemp;
    using type_map::Find;
    using type_map::ContainsKey;
    using type_map::TypbootmempEntry;

} // namespace ustl

#endif // #ifndef USTL_UTIL_TYPES_MAP_H