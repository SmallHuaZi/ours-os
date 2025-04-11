/// This header define a group of common type algorithm interfaces.
///
#ifndef USTL_UTIL_TYPE_ALGOS_HPP
#define USTL_UTIL_TYPE_ALGOS_HPP

#include <ustl/config.hpp>
#include <ustl/limits.hpp>
#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/is_same.hpp>

namespace ustl {
    struct TypeAlgos {
        struct NotFound;

        USTL_CXX11_CONSTEXPR
        static isize const kNotFound = ustl::NumericLimits<isize>::max();

        template <typename This, usize I>
        struct DoAt;

        template <typename This, usize I>
        using At = typename DoAt<This, I>::Type;

        template <typename This, typename Object>
        struct DoGetPos;

        template <typename This, typename Object>
        static usize const GetPosV = DoGetPos<This, Object>::VALUE;

        template <typename This, typename Object>
        struct DoFind;

        template <typename This, typename Object>
        using Find = typename DoFind<This, Object>::Type;

        template <typename This, typename Object>
        struct DoContains 
            : traits::BoolConstant<!traits::IsSameV<Find<This, Object>, NotFound>>
        {};

        template <typename This, typename Object>
        static bool const ContainsV = DoContains<This, Object>();

        template <typename This, typename... Object>
        struct DoPushFront;

        template <typename This, typename... Object>
        using PushFront = typename DoPushFront<This, Object...>::Type;

        template <typename This, typename... Object>
        struct DoPushBack;

        template <typename This, typename... Object>
        using PushBack = typename DoPushBack<This, Object...>::Type;

        template <typename This>
        struct DoPopFront;

        template <typename This>
        using PopFront = typename DoPopFront<This>::Type;

        template <typename This>
        struct DoPopBack;

        template <typename This>
        using PopBack = typename DoPopBack<This>::Type;

        template <typename This>
        struct DoInvert;

        template <typename This>
        using Invert = typename DoInvert<This>::Type;

        template <typename This, typename Other>
        struct DoMerge;

        template <typename This, typename Other>
        using Merge = typename DoMerge<This, Other>::Type;

        template <typename This>
        struct DoUnique;

        template <typename This>
        using Unique = typename DoUnique<This>::Type;

        template <typename This, usize Pos>
        struct DoTruncate;

        template <typename This, usize Pos>
        using Truncate = typename DoTruncate<This, Pos>::Type;

        template <typename This, usize Start, usize End>
        struct DoRange;

        template <typename This, usize Start, usize End>
        using Range = typename DoRange<This, Start, End>::Type;

        template <typename This>
        struct DoSize;

        template <typename This>
        static usize const SizeV = DoSize<This>();

        template <typename This, template <typename> typename F>
        struct DoTransform;

        template <typename This, template <typename> typename F>
        using Transform = typename DoTransform<This, F>::Type;
    };

} // namespace ustl

#endif // USTL_UTIL_TYPE_ALGOS_HPP
