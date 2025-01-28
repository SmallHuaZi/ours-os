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

#ifndef USTL_COLLECTIONS_VEC_HPP
#define USTL_COLLECTIONS_VEC_HPP 1

#include <boost/container/vector.hpp>

namespace ustl::collections {
    template <typename T, typename A>
    class Vec
        : public ::boost::container::vector<T, A>
    {
        typedef ::boost::container::vector<T, A>        Base;
    public:
        typedef typename Base::value_type               Element;
        typedef typename Base::pointer                  PtrMut;
        typedef typename Base::const_pointer            Ptr;
        typedef typename Base::reference                RefMut;
        typedef typename Base::const_reference          Ref;
        typedef typename Base::iterator                 IterMut;
        typedef typename Base::const_iterator           Iter;
        typedef typename Base::reverse_iterator         RevIterMut;
        typedef typename Base::const_reverse_iterator   RevIter;
        typedef typename Base::allocator_type           AllocatorType;
        typedef typename Base::allocator_traits         AllocatorTraits;

        using Base::Base;

        Vec(PtrMut pointer, size_t capacity, AllocatorType &&allocator)
            : Base(boost::container::initial_capacity_t{}, pointer, capacity, allocator)
        {}
    };

} // namespace ustl::collections

#endif // #ifndef USTL_COLLECTIONS_VEC_HPP