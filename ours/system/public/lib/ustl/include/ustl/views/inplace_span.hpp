#ifndef USTL_VIEWS_INPLACE_SPAN_HPP
#define USTL_VIEWS_INPLACE_SPAN_HPP

#include <ustl/config.hpp>
#include <ustl/traits/ref.hpp>

#include <ustl/iterator/reverse_iterator.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

namespace ustl::views {
    template <typename T>
    struct InplaceSpan {
        static_assert(!ustl::traits::IsRefV<T>, "Storing a references is not allowed!");

        typedef T   Element;
        typedef T *         PtrMut;
        typedef T const *   Ptr;
        typedef T &         RefMut;
        typedef T const &   Ref;

        typedef PtrMut  IterMut;
        typedef Ptr     Iter;
        typedef iterator::RevIter<IterMut>  RevIterMut;
        typedef iterator::RevIter<Iter>     RevIter;

        USTL_FORCEINLINE USTL_CONSTEXPR
        InplaceSpan() = default;

        USTL_FORCEINLINE USTL_CONSTEXPR
        InplaceSpan(usize n)
            : count(n)
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto begin() -> IterMut {
            return elements;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto end() -> IterMut {
            return elements + count;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto begin() const -> Iter {
            return elements;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto end() const -> Iter {
            return elements + count;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto rbegin() -> RevIterMut {
            return end();
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto rend() -> IterMut {
            return begin();
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto reset(usize n) -> void {
            count = n;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator[](usize i) -> T & {
            DEBUG_ASSERT(i < count);
            return elements[i];
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator[](usize i) const -> T const & {
            DEBUG_ASSERT(i < count);
            return elements[i];
        }

        usize count;
        T elements[];
    };
    static_assert(sizeof(InplaceSpan<usize>) == sizeof(usize), "InplaceSpan is too large");

} // namespace ustl::views

#endif // USTL_VIEWS_INPLACE_SPAN_HPP