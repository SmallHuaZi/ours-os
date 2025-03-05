// ustl/iterator/details USTL/ITERATOR/DETAILS_ITER_TYPES_HPP
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

#ifndef USTL_ITERATOR_DETAILS_ITER_TYPES_HPP
#define USTL_ITERATOR_DETAILS_ITER_TYPES_HPP 1

#include <ustl/traits/cv.hpp>
#include <ustl/traits/conditional.hpp>

#include <ustl/util/pack_options.hpp>

namespace ustl::iter {
namespace details {
    USTL_CONSTANT_OPTION(ReadOnly, bool, VALUE);

    template <typename... Options>
    struct IteratorTypes;

    template <bool Const, typename TraitsType>
    struct IteratorTypes<ReadOnly<Const>, TraitsType>
    {
        typedef TraitsType  Traits;
        typedef isize       Distance;
        typedef typename Traits::Element Element;
        typedef typename Traits::Ptr     Ptr;
        typedef typename Traits::PtrMut  PtrMut;
        typedef typename Traits::Ref     Ref;
        typedef typename Traits::RefMut  RefMut;

        typedef traits::ConditionalT<Const, Ref, RefMut>    SafeRef;
        typedef traits::ConditionalT<Const, Ptr, PtrMut>    SafePtr;
    };

} // namespace ustl::iter::details
} // namespace ustl::iter


#endif // #ifndef USTL_ITERATOR_DETAILS_ITER_TYPES_HPP