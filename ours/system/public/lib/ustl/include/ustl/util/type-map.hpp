#ifndef USTL_UTIL_TYPES_MAP_H
#define USTL_UTIL_TYPES_MAP_H

#include <ustl/config.hpp>
#include <ustl/traits/conditional.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/util/type-algos.hpp>

namespace ustl {
    template <typename Key, typename Val>
    struct KeyValPair {
        typedef Key KeyType;
        typedef Val ValType;
    };

    template <typename KeyValPair>
    struct Entry {
        typedef typename KeyValPair::KeyType KeyType;
        typedef typename KeyValPair::ValType ValType;
    };

    template <typename Key, typename Val>
    using TypeMapEntry = Entry<KeyValPair<Key, Val>>;

    template <typename... Entries>
    struct TypeMapImpl;

    // Specilization for correct usage.
    template <typename... KvPairs>
    struct TypeMapImpl<Entry<KvPairs>...> {};

    template <typename... KvPairs>
    using TypeMap = TypeMapImpl<Entry<KvPairs>...>;

    template <typename Key, typename... KvPairs>
    struct TypeAlgos::DoFind<TypeMap<KvPairs...>, Key> {
        template <usize I, typename... OtherKvPairs>
        struct FindImpl;

        template <typename KvPair>
        struct FindImpl<0, Key, TypeMap<KvPair>> {
            typedef typename KvPair::KeyType    CurrentKey;
            typedef traits::ConditionalT<traits::IsSameV<Key, CurrentKey>, typename KvPair::ValType, NotFound> Type;
        };

        template <usize I, typename HeadPair, typename... TailPairs>
        struct FindImpl<I, Key, TypeMap<HeadPair, TailPairs...>> {
            typedef typename HeadPair::KeyType CurrentKey;

            typedef traits::ConditionalT<traits::IsSameV<Key, CurrentKey>, 
                    typename HeadPair::ValType,
                    typename FindImpl<I - 1, Key, TypeMap<TailPairs...>>::Type
            > Type;
        };

        typedef typename FindImpl<sizeof...(KvPairs) - 1, Key, TypeMap<KvPairs...>>::Type Type;
    };

    template <typename T, typename... KvPairs>
    struct TypeAlgos::DoAt<TypeMap<T, KvPairs...>, 0> {
        typedef T   Type;
    };

    template <usize I, typename T, typename... KvPairs>
    struct TypeAlgos::DoAt<TypeMap<T, KvPairs...>, I> {
        static_assert(sizeof...(KvPairs) + 1 != 0, "[ustl-error] Unsupport empty types map.\n");
        typedef TypeAlgos::At<TypeMap<KvPairs...>, I - 1>   Type;
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_TYPES_MAP_H