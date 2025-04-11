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

#ifndef USTL_ALGORITHMS_MOD_HPP
#define USTL_ALGORITHMS_MOD_HPP 1

#include <algorithm>
#include <boost/algorithm/algorithm.hpp>

namespace ustl::algorithms {
    using boost::algorithm::power;
    using std::all_of;
    using std::none_of;
    using std::any_of;

    /// Copy and Move
    using std::copy;
    using std::copy_n;
    using std::copy_if;
    using std::copy_backward;
    using std::partition_copy;
    using std::move;
    using std::move_backward;
    using std::move_if_noexcept;

    using std::uninitialized_copy;
    using std::uninitialized_move;
    using std::uninitialized_copy_n;
    using std::uninitialized_move_n;

    /// Change on sequence 
    using std::reverse;
    using std::reverse_copy;
    using std::rotate;
    using std::rotate_copy;

    /// Merge
    using std::merge;
    using std::inplace_merge;

    /// Sort and Partition 
    using std::sort;
    using std::stable_sort;
    using std::partial_sort;
    using std::partial_sort_copy;
    using std::is_sorted;
    using std::is_sorted_until;

    using std::partition;
    using std::stable_partition;
    using std::is_partitioned;

    /// Count
    using std::count;
    using std::count_if;
    using std::nth_element;

    /// Transform
    using std::transform;
    using std::replace;
    using std::replace_copy;

    /// Binary search
    using std::binary_search;
    using std::lower_bound;
    using std::upper_bound;
    using std::equal_range;

    /// Compare
    using std::lexicographical_compare;
    using std::lexicographical_compare_three_way;

    /// Maximum and Minimum 
    using std::min; 
    using std::max; 
    using std::minmax; 
    using std::min_element; 
    using std::max_element; 
    using std::minmax_element; 
    using std::clamp; 

    /// Heap
    using std::make_heap;
    using std::push_heap;
    using std::pop_heap;
    using std::sort_heap;
    using std::is_heap;
    using std::is_heap_until;

    using std::swap;

} // namespace ustl::algorithms

#endif // #ifndef USTL_ALGORITHMS_MOD_HPP