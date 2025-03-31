// ustl/algorithms USTL/ALGORITHMS_SORT_HPP
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

#ifndef USTL_ALGORITHMS_SORT_HPP
#define USTL_ALGORITHMS_SORT_HPP 1

#include <algorithm>

namespace ustl::algorithms {
    using std::is_sorted;
    using std::is_sorted_until;
    using std::sort;
    using std::stable_sort;
    using std::partial_sort;
    using std::partial_sort_copy;
    using std::nth_element;

} // namespace ustl::algorithms

#endif // #ifndef USTL_ALGORITHMS_SORT_HPP