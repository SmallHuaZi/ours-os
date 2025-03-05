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

#ifndef USTL_ITERATOR_TRAITS_HPP
#define USTL_ITERATOR_TRAITS_HPP 1

#include <ustl/iterator/category.hpp>

#include <ustl/traits/void.hpp>
#include <ustl/traits/function_detector.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace ustl::iter {
namespace details {
    template <typename T, typename = void>
    struct IterTraitsBase
    {};

    template <typename T>
    struct IterTraitsBase<T, traits::VoidT<
            typename T::pointer,
            typename T::reference,
            typename T::iter_tag,
            typename T::difference_type,
            typename T::value_type>>
    {
        typedef typename T::pointer             SafePtr;
        typedef typename T::reference           SafeRef;
        typedef typename T::value_type          Element;
        typedef typename T::difference_type     Distance;
        typedef typename T::iterator_category   Category;
    };

    // template <typename T>
    // struct IterTraitsBase<T, traits::VoidT<
    //         typename T::SafePtr,
    //         typename T::SafeRef,
    //         typename T::Category,
    //         typename T::Distance,
    //         typename T::Element>>
    // {
    //     typedef typename T::SafePtr    SafePtr;
    //     typedef typename T::SafeRef    SafeRef;
    //     typedef typename T::Element    Element;
    //     typedef typename T::Distance   Distance;
    //     typedef typename T::Category   Category;
    // };

    template <typename T>
    struct IterTraitsBase<T *>
    {
        typedef T *         SafePtr;
        typedef T &         SafeRef;
        typedef T           Element;
        typedef isize       Distance;
        typedef BidirectionIteratorTag Category;
    };

    template <typename Iterator>
    struct InputIterTraits
        : public IterTraitsBase<Iterator> 
    {
        typedef IterTraitsBase<Iterator> Base;
        typedef InputIterTraits          Self;

        typedef typename Base::SafePtr      SafePtr;
        typedef typename Base::SafeRef      SafeRef;
        typedef typename Base::Category     Category;
        typedef typename Base::Element      Element; 
        typedef typename Base::Distance     Distance; 

        static_assert(traits::HasFnInc<Iterator &(Iterator::*)()>::VALUE,  "Cannot iter.operator++()");
        static_assert(traits::HasFnInc<Iterator (Iterator::*)(int)>::VALUE, "Cannot iter.operator++(int)");
        static_assert(traits::HasFnDef<SafeRef (Iterator::*)()>::VALUE, "Cannot iter.operator*()");
        static_assert(traits::HasFnArrow<SafeRef (Iterator::*)()>::VALUE, "Cannot iter.operator->()");
    };

    template <typename Iterator>
    struct ForwardIterTraits
        : public InputIterTraits<Iterator> 
    {
        typedef ForwardIterTraits          Self;
        typedef InputIterTraits<Iterator>  Base;

        typedef typename Base::SafePtr      SafePtr;
        typedef typename Base::SafeRef      SafeRef;
        typedef typename Base::Category     Category;
        typedef typename Base::Element      Element;
        typedef typename Base::Distance     Distance;
    };

    template <typename Iterator>
    struct BidirectionIterTraits
        : public ForwardIterTraits<Iterator> 
    {
        typedef BidirectionIterTraits        Self;
        typedef ForwardIterTraits<Iterator>  Base;

        typedef typename Base::SafePtr      SafePtr;
        typedef typename Base::SafeRef      SafeRef;
        typedef typename Base::Category     Category;
        typedef typename Base::Element      Element;
        typedef typename Base::Distance     Distance;

        static_assert(traits::HasFnDec<Iterator (Iterator::*)(int)>::VALUE,  "Cannot iter.operator--(int)");
        static_assert(traits::HasFnDec<Iterator &(Iterator::*)()>::VALUE,  "Cannot iter.operator--()");
    };

    template <typename Iterator>
    struct RandomAccessIterTraits
        : public InputIterTraits<Iterator> 
    {
        typedef RandomAccessIterTraits           Self;
        typedef BidirectionIterTraits<Iterator>  Base;

        typedef typename Base::SafePtr      SafePtr;
        typedef typename Base::SafeRef      SafeRef;
        typedef typename Base::Category     Category;
        typedef typename Base::Element      Element;
        typedef typename Base::Distance     Distance;

        static_assert(traits::HasFnAdd<Iterator (Iterator::*)(Distance)>::VALUE,  "Cannot iter.operator-(int)");
        static_assert(traits::HasFnSub<Iterator (Iterator::*)(Distance)>::VALUE,  "Cannot iter.operator+(int)");
        static_assert(traits::HasFnAddAssign<Iterator &(Iterator::*)(Distance)>::VALUE,  "Cannot iter.operator-=(isize)");
        static_assert(traits::HasFnSubAssign<Iterator &(Iterator::*)(Distance)>::VALUE,  "Cannot iter.operator+=(isize)");
        static_assert(traits::HasFnSubAssign<Iterator &(Iterator::*)(Distance)>::VALUE,  "Cannot iter.operator[](usize)");
    };

} // namespace ustl::iter::details

    template <typename T>
    struct IterTraits
        : public details::IterTraitsBase<T>
    {};

    template <typename T>
    using CategoryT = typename IterTraits<T>::Category; 
    
} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_TRAITS_HPP