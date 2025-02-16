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

#ifndef USTL_COLLECTIONS_DEQUE_HPP
#define USTL_COLLECTIONS_DEQUE_HPP 1

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>

namespace ustl::collections {
    template <size_t N>
    using BlockSize = ::boost::container::block_size<N>;

    template <size_t N>
    using BlockBytes = ::boost::container::block_bytes<N>;

    /// Supported options are: \c BlockBytes and \c BlockSize
    template <typename... Options>
    using DequeOptions = ::boost::container::deque_options<Options...>;

    template <typename T, typename A, typename Options>
    class Deque 
        : public ::boost::container::deque<T, A, Options>
    {
        typedef ::boost::container::deque<T, A, Options>   Base;
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
        typedef typename Base::allocator_traits_t       AllocatorTraits;

        using Base::Base;
    };

} // namespace ustl::collections

#endif // #ifndef USTL_COLLECTIONS_DEQUE_HPP