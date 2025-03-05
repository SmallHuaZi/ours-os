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

#ifndef USTL_COLLECTIONS_PMR_VEC_HPP
#define USTL_COLLECTIONS_PMR_VEC_HPP 1

#include <boost/container/pmr/vector.hpp>
#include <ustl/collections/pmr/memory_resource.hpp>

namespace ustl::collections::pmr {
    template <typename T>
    struct Vec 
        : public ::boost::container::pmr::vector<T>
    {
        typedef ::boost::container::pmr::vector<T>      Base;

        typedef typename Base::value_type               Element;
        typedef typename Base::pointer                  PtrMut;
        typedef typename Base::const_pointer            Ptr;
        typedef typename Base::reference                RefMut;
        typedef typename Base::const_reference          Ref;
        typedef typename Base::iterator                 IterMut;
        typedef typename Base::const_iterator           Iter;
        typedef typename Base::reverse_iterator         RevIterMut;
        typedef typename Base::const_reverse_iterator   RevIter;

        using Base::Base;
    };

} // namespace ustl::collections::pmr

#endif // #ifndef USTL_COLLECTIONS_PMR_VEC_HPP