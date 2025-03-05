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

#ifndef USTL_UTIL_INDEX_SEQUENCE_HPP
#define USTL_UTIL_INDEX_SEQUENCE_HPP 1

#include <ustl/config.hpp>

/// @example
/// 1. MakeIndexSequence<5> will be split into two halves: MakeIndexSequence<2> and MakeIndexSequence<3>.
/// 
/// 2. MakeIndexSequence<2> will also be split into two halves: MakeIndexSequence<1> and MakeIndexSequence<1>.
/// 
/// 3. MakeIndexSequence<1> is a base case that generates a sequence IndexSequence<0>.
/// 
/// 4. MakeIndexSequence<1> is also a base case and generates a sequence IndexSequence<0>.
/// 
/// 5. Next, the two sequences are merged and renumbered. MergeAndRenumber's job is to concatenate two sequences 
///    and renumber the ValueTypes of the second sequence.
/// 
/// 6. IndexSequence<0> and IndexSequence<0> merge to become IndexSequence<0, 0>. Since the size of the second sequence 
///    is 1, its ValueTypes need to be renumbered by adding the value of sizeof...(N1). Here, the value of sizeof...(N1) is 1, 
///    so ValueType 0 of the second sequence is renumbered to 1. Ultimately, the result is an IndexSequence<0, 1> sequence of 
///    integers from 0 to 1. The final step is to merge the two halves of MakeIndexSequence<2>. MergeAndRenumber concatenates 
///    the two sequences and renumbers the ValueTypes of the second sequence.
/// 
/// 7. IndexSequence<0, 1> and IndexSequence<0, 1> merge to become IndexSequence<0, 1, 0, 1>. The size of the second sequence 
///    is 2, so its ValueTypes need to be renumbered by adding the value of sizeof...(N1). Here, the value of sizeof...(N1) is 2, 
///    so ValueTypes 0 and 1 of the second sequence are renumbered to 2 and 3. Ultimately, the result is an IndexSequence<0, 1, 2, 3> 
///    containing a sequence of integers from 0 to 3.Finally, you have an IndexSequence<0, 1, 2, 3, 4> containing the sequence of 
///    integers from 0 to 4.

namespace ustl {
    /// @brief 
    /// @tparam Integer 
    /// @tparam ...N 
    template <typename Integer, Integer... N>
    struct IntegerSequence
    {
        typedef Integer          Element;
        typedef IntegerSequence  Type;

        USTL_FORCEINLINE USTL_CONSTEXPR 
        static auto size() -> Element
        {  return sizeof...(N);  }
    };

    template <usize... N>
    using IndexSequence = IntegerSequence<usize, N...>;


    template <typename Sequence1, typename Sequence2>
    struct MergeAndRenumber;

    template <usize... N1, usize... N2>
    struct MergeAndRenumber<IndexSequence<N1...>, IndexSequence<N2...>>
        : IndexSequence<N1..., (sizeof...(N1) + N2)...>
    {};


    template <usize N>
    struct MakeIndexSequence
        : MergeAndRenumber<typename MakeIndexSequence<N / 2>::Type,
                           typename MakeIndexSequence<N - N / 2>::Type>
    {};

    template<> struct MakeIndexSequence<0>: IndexSequence<> {};
    template<> struct MakeIndexSequence<1>: IndexSequence<0> {};

    template <usize Start, usize Finish>
    struct MakeRangeSequence
    {};

    template <usize Start>
    struct MakeRangeSequence<Start, Start>
    {};

} // namespace ustl

#endif // #ifndef USTL_UTIL_INDEX_SEQUENCE_HPP