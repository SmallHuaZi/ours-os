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
#ifndef USTL_UTIL_VISIT_HPP
#define USTL_UTIL_VISIT_HPP 1

#include <ustl/config.hpp>
#include <ustl/util/move.hpp>
#include <ustl/util/index_sequence.hpp>
#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/invoke_result.hpp>

namespace ustl {
    template <typename T>
    struct SizeOf;

    template <typename T, usize I>
    struct Getter;

    template <typename Visitor, typename T, typename Indexies>
    struct Visit; 

    template <typename Visitor, typename T>
    struct Visit<Visitor, T, IndexSequence<>>
    {
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator()(Visitor &&visitor, T &&t) -> void {}
    };

    template <typename Visitor, typename T, usize I, usize... Indexies>
    struct Visit<Visitor, T, IndexSequence<I, Indexies...>>
        : public Visit<Visitor, T, IndexSequence<Indexies...>>
    {
        typedef Visit<Visitor, T, IndexSequence<Indexies...>>   Base;
        typedef Getter<T, I>                Getter;
        typedef typename Getter::RetType    RetType;

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto visit(Visitor &&visitor, T &&t) -> void {
            Getter get;
            visitor(ustl::forward<RetType>(get(t)));
            Base::visit(ustl::forward<RetType>(visitor), ustl::forward<T>(t));
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator()(Visitor &&visitor, T &&t) -> void {
            visit(ustl::forward<RetType>(visitor), ustl::forward<T>(t));
        }
    };

    /// `ustl::SizeOf<T>()` and `ustl::get<I>()` should be override for `T`.
    template <typename Visitor, typename T>
    USTL_CONSTEXPR
    auto visit(Visitor &&visitor, T &&t) -> void {
        typedef typename MakeIndexSequence<SizeOf<T>::VALUE>::Type Indexies;
        typedef Visit<Visitor, T, Indexies> Visit;
        Visit()(ustl::forward<Visitor>(visitor), ustl::forward<T>(t));
    }

} // namespace ustl

#endif // #ifndef USTL_UTIL_VISIT_HPP