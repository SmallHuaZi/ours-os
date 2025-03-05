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

#ifndef USTL_ITERATOR_DISTANCE_OF_HPP
#define USTL_ITERATOR_DISTANCE_OF_HPP 1

#include <ustl/iterator/traits.hpp>

namespace ustl::iter {
    template <typename Iterator>
    struct IterDistanceOf
    {
        typedef typename IterTraits<Iterator>::Distance     Type;
    };

    template <typename... Iterators>
    using IterDistanceOfT = typename IterDistanceOf<Iterators...>::Type;

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_DISTANCE_OF_HPP