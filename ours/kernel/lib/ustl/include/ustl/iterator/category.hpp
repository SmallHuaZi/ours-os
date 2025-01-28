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

#ifndef USTL_ITERATOR_CATEGORY_HPP
#define USTL_ITERATOR_CATEGORY_HPP 1

#include <ustl/config.hpp>
#include <ustl/traits/is_base_of.hpp>

namespace ustl::iter {
    struct InputIteratorTag 
    {};

    struct OutputIteratorTag 
    {};

    struct ForwardIteratorTag
        : public InputIteratorTag
    {};

    struct BidirectionIteratorTag
        : public ForwardIteratorTag
    {};

    struct RandomAccessIteratorTag
        : public BidirectionIteratorTag
    {};

    struct ContiguousIteratorTag
        : public RandomAccessIteratorTag
    {};

    template <typename T, typename U>
    struct CategorysMatcher 
    { USTL_CONSTEXPR static bool const VALUE = traits::IsBaseOfV<T, U>; };

    template <typename T>
    struct IsInputIter: CategorysMatcher<T, InputIteratorTag> {};

    template <typename T>
    struct IsForwardIter: CategorysMatcher<T, ForwardIteratorTag> {};
    
    template <typename T>
    struct IsBidirectionIter: CategorysMatcher<T, BidirectionIteratorTag> {};
    
    template <typename T>
    struct IsRandomAccessIter: CategorysMatcher<T, RandomAccessIteratorTag> {};
    
    template <typename T>
    struct IsContiguousIter: CategorysMatcher<T, ContiguousIteratorTag> {};

    template <typename T>
    USTL_CONSTEXPR 
    bool const IsInputIterV = IsInputIter<T>::VALUE;

    template <typename T>
    USTL_CONSTEXPR 
    bool const IsForwardIterV = IsForwardIter<T>::VALUE;

    template <typename T>
    USTL_CONSTEXPR 
    bool const IsBidirectionIterV = IsBidirectionIter<T>::VALUE;

    template <typename T>
    USTL_CONSTEXPR 
    bool const IsRandomAccessIterV = IsRandomAccessIter<T>::VALUE;

    template <typename T>
    USTL_CONSTEXPR 
    bool const IsContiguousIterV = IsContiguousIter<T>::VALUE;

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_CATEGORY_HPP