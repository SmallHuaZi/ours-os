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

#ifndef USTL_ITERATOR_IS_ITER_HPP
#define USTL_ITERATOR_IS_ITER_HPP 1

#include <ustl/iterator/traits.hpp>
#include <ustl/util/type_map.hpp>
#include <ustl/traits/is_same.hpp>

namespace ustl::iter {
namespace details {

    template <typename T, typename = void>
    struct StdIterTraits
        : traits::FalseType
    {};
    
    template <typename T>    
    struct StdIterTraits<T, traits::VoidT<
                         typename T::pointer,
                         typename T::reference,
                         typename T::value_type,
                         typename T::difference_type,
                         typename T::iterator_category>>
        : traits::TrueType
    {
        typedef typename T::pointer             SafePtr;
        typedef typename T::reference           SafeRef;
        typedef typename T::value_type          Element;
        typedef typename T::difference_type     Distance;
        typedef typename T::iterator_category   Category;
    };
    
    template <typename T, typename = void>
    struct DefaultIterTraits
        : traits::FalseType
    {};
    
    template <typename T>    
    struct DefaultIterTraits<T, traits::VoidT<
                         typename T::SafePtr,
                         typename T::SafeRef,
                         typename T::Element,
                         typename T::Distance,
                         typename T::Category>>
        : traits::TrueType
    {};
    
    // Todo!!!
    // Implements all iterator interfaces but is unidentifiable iterator.
    template <typename Iterator>
    struct IsIterable
        : traits::FalseType
    {};
    
    // c++ standard iterator.
    template <typename Iterator>
    struct IsStdIter
    {
        static bool const VALUE = StdIterTraits<Iterator>::VALUE;
    };
    
    template <typename Iterator>
    struct IsDefaultIter
    {
        static bool const VALUE = DefaultIterTraits<Iterator>::VALUE;
    };
    
    template <typename Iterator>
    struct IsIter
    {
        static bool const VALUE = IsStdIter<Iterator>::VALUE || IsDefaultIter<Iterator>::VALUE;
    };
    
    template <typename T>
    struct IsIter<T *>
    {
        static bool const VALUE = true;
    };
    
} // namespace ustl::iter::details 
    
    template <typename T>
    USTL_CONSTEXPR 
    bool const IsIterV = details::IsIter<T>::VALUE;
    
    template <typename T>
    USTL_CONSTEXPR 
    bool const IsStdIterV = details::IsStdIter<T>::VALUE;

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_IS_ITER_HPP