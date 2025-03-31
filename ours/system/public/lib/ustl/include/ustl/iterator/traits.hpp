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

#include <ustl/iterator/category.hpp>

#include <ustl/traits/void.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/conditional.hpp>
#include <ustl/traits/integral_constant.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/cv.hpp>

namespace ustl::iterator {
namespace details {
    template <typename T, typename = void>
    struct StdIteratorTraits
        : traits::FalseType
    {};

    template <typename T>
    struct StdIteratorTraits<T, traits::VoidT<
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
    struct DefaultIteratorTraits
        : traits::FalseType
    {};

    template <typename T>
    struct DefaultIteratorTraits<T, traits::VoidT<
                         typename T::Pointer,
                         typename T::Reference,
                         typename T::Element,
                         typename T::Distance,
                         typename T::Category>>
        : traits::TrueType
    {
        typedef typename T::Pointer             Pointer;
        typedef typename T::Reference           Reference;
        typedef typename T::Element             Element;
        typedef typename T::Distance            Distance;
        typedef typename T::Category            Category;
    };

    template <typename T>
    struct DefaultIteratorTraits<T *>
        : traits::TrueType
    {
        typedef traits::RemoveCvT<T>       Element;
        typedef T *     Pointer;
        typedef T &     Reference;
        typedef isize   Distance;
        typedef BidirectionIteratorTag  Category;
    };

    // Todo!!!
    // Implements all iterator interfaces but is unidentifiable iterator.
    template <typename Iterator>
    struct IsIterable
        : traits::FalseType
    {};

    // c++ standard iterator.
    template <typename Iterator>
    USTL_CONSTEXPR
    static bool const IsStdIteratorV = StdIteratorTraits<Iterator>();

    template <typename Iterator>
    static bool const IsDefaultIteratorV = DefaultIteratorTraits<Iterator>();

    template <typename Iterator>
    struct IsIterator
        : traits::BoolConstant<IsStdIteratorV<traits::RemoveCvRefT<Iterator>> || 
                               IsDefaultIteratorV<traits::RemoveCvRefT<Iterator>>>
    {};

    template <typename T>
    struct IsIterator<T *>
        : traits::TrueType
    {};

    template <typename Iterator>
    struct MakeIteratorTratis {
        typedef traits::ConditionalT<IsStdIteratorV<Iterator>, 
            StdIteratorTraits<Iterator>, 
            DefaultIteratorTraits<Iterator>
        > Type;
    };

} // namespace ustl::iterator::details

    using details::IsStdIteratorV;

    template <typename T>
    USTL_CONSTEXPR
    bool const IsIteratorV = details::IsIterator<T>::VALUE;

    template <typename Iterator>
    using IterTraits = typename details::MakeIteratorTratis<Iterator>::Type;

    template <typename Iterator>
    using CategoryT = typename IterTraits<Iterator>::Category; 

    template <typename Iterator>
    using ElementOfT = typename IterTraits<Iterator>::Element;

    template <typename Iterator>
    using DistanceOfT = typename IterTraits<Iterator>::Distance;

} // namespace ustl::iterator

#endif // #ifndef USTL_ITERATOR_IS_ITER_HPP
