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

#ifndef USTL_ALGORITHMS_SEARCH_HPP
#define USTL_ALGORITHMS_SEARCH_HPP 1

#include <algorithm>

namespace ustl::algorithms {
    using std::any_of;
    using std::all_of;
    using std::none_of;

    using std::find;
    using std::find_if;
    using std::find_if_not;
    using std::find_end;
    using std::find_first_of;
    using std::adjacent_find;

    using std::count;
    using std::count_if;

    using std::mismatch;
    using std::equal;
    using std::search;
    using std::search_n;

    using std::lower_bound;
    using std::upper_bound;
    using std::binary_search;

} // namespace ustl::algorithms

#endif // #ifndef USTL_ALGORITHMS_SEARCH_HPP