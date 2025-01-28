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

#ifndef USTL_COLLECTIONS_ARRAY_HPP
#define USTL_COLLECTIONS_ARRAY_HPP 1

#include <boost/container/static_vector.hpp>

namespace ustl::collections {
    typedef boost::container::static_vector_options_t
        < 
            // Disabling this namely 'throw_on_overflow<false>`, it will 
            // forbid c++ exception system.
            boost::container::throw_on_overflow<false>
        > StaticVecOptions;
    
    template <typename T, std::size_t Capacity>
    struct Array 
        : public ::boost::container::static_vector<T, Capacity, StaticVecOptions>
    {
        typedef ::boost::container::static_vector<T, Capacity, StaticVecOptions>    Base;
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

} // namespace ours

#endif // #ifndef USTL_COLLECTIONS_ARRAY_HPP