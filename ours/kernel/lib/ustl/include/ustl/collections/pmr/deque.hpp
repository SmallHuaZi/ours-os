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

#ifndef USTL_COLLECTIONS_PMR_DEQUE_HPP
#define USTL_COLLECTIONS_PMR_DEQUE_HPP 1

#include <boost/container/pmr/deque.hpp>

namespace ustl::collections::pmr {
    using namespace boost::container::pmr;

    template <typename T>
    using Deque = boost::container::pmr::deque<T>;

} // namespace ustl::collections::pmr

#endif // #ifndef USTL_COLLECTIONS_PMR_DEQUE_HPP