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

#ifndef USTL_UTIL_FOLD_HPP
#define USTL_UTIL_FOLD_HPP 1

#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl::util {
    /// Fold<Handler, Accumulator, Objects...>
    ///
    /// Folds every `|Object|` into an `|Accumulator|` by applying `|Handler|`, returning the final result.  
    template <template <typename A> typename Handler, 
              template <typename V1, typename V2> typename Accumulator,
              typename... Objects>
    struct Fold;

    template <template <typename A> typename Handler, 
              template <typename V1, typename V2> typename Accumulator,
              typename HeadObject, 
              typename... RemainingObjects> 
    struct Fold<Handler, Accumulator, HeadObject, RemainingObjects...> {
        typedef Fold<Handler, Accumulator, RemainingObjects...>  NextIteration;

        typedef typename Handler<HeadObject>::RetType   V1;
        typedef typename NextIteration::RetType         V2;
        typedef typename Accumulator<V1, V2>::RetType   RetType;

        USTL_CONSTEXPR
        static RetType const VALUE = Accumulator<V1, V2>::template Accumulate<
                                        Handler<HeadObject>::VALUE,
                                        NextIteration::VALUE
                                    >::VALUE;
    };

    // Ony one object, just calculates it and return result. `Accumulator` is unused here.
    template <template <typename A> typename Handler, 
              template <typename V1, typename V2> typename Accumulator,
              typename Object> 
    struct Fold<Handler, Accumulator, Object>
    {
        typedef typename Handler<Object>::RetType   RetType;

        USTL_CONSTEXPR
        static RetType const VALUE = Handler<Object>::VALUE;
    };

    /// Provides a of pre-defined group accumulator
    struct Accumulator
    {
        template <typename, typename>
        struct BoolOr
        {
            typedef bool    RetType;
            template <bool V1, bool V2>
            using Accumulate = traits::BoolConstant<V1 || V2>;
        };

        template <typename, typename>
        struct BoolAnd
        {
            typedef bool    RetType;
            template <bool V1, bool V2>
            using Accumulate = traits::BoolConstant<V1 && V2>;
        };

        template <typename, typename>
        struct IntAdd
        {
            typedef isize   RetType;
            template <isize V1, isize V2>
            using Accumulate = traits::IntegralConstant<isize, V1 + V2>;
        };

        template <typename, typename>
        struct IntSub
        {
            typedef isize   RetType;
            template <isize V1, isize V2>
            using Accumulate = traits::IntegralConstant<isize, V1 - V2>;
        };

        template <typename, typename>
        struct IntMul
        {
            typedef isize   RetType;
            template <isize V1, isize V2>
            using Accumulate = traits::IntegralConstant<isize, V1 * V2>;
        };

        template <typename, typename>
        struct IntDiv
        {
            typedef isize   RetType;
            template <isize V1, isize V2>
            using Accumulate = traits::IntegralConstant<isize, V1 / V2>;
        };
    };

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_FOLD_HPP