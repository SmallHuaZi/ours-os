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
    template <typename T, std::size_t E = boost::dynamic_extent>
    struct Span
        : public ::boost::span<T, E>
    {
        typedef ::boost::span<T>    Base;
        using Base::Base;
        using Base::operator=;

        template <typename U, std::size_t UE = boost::dynamic_extent>
        Span(::boost::span<U, UE> other_span)
            : Base(other_span)
        {}

        typedef typename Base::value_type       Element;
        typedef typename Base::iterator         IterMut;
        typedef typename Base::const_iterator   Iter;
        typedef typename Base::reverse_iterator         RevIterMut;
        typedef typename Base::const_reverse_iterator   RevIter;
    };

    template <typename T, std::size_t E = boost::dynamic_extent>
    constexpr auto make_span(T *t, std::size_t n) -> Span<T, E> {
        return {t, n};
    }

} // namespace ustl::views

#endif // #ifndef USTL_VIEWS_SPAN_HPP