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

#ifndef USTL_VIEWS_SPAN_HPP
#define USTL_VIEWS_SPAN_HPP 1

#include <boost/core/span.hpp>

namespace ustl::views {
    template <typename T>
    struct Span
        : public ::boost::span<T>
    {
        typedef ::boost::span<T>    Base;
        using Base::Base;

        typedef typename Base::iterator         IterMut;
        typedef typename Base::const_iterator   Iter;
        typedef typename Base::reverse_iterator         RevIterMut;
        typedef typename Base::const_reverse_iterator   RevIter;
    };

} // namespace ustl::views

#endif // #ifndef USTL_VIEWS_SPAN_HPP