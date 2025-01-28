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

#ifndef USTL_ALGORITHMS_PARTITION_HPP
#define USTL_ALGORITHMS_PARTITION_HPP 1

#include <algorithm>

namespace ustl::algorithms {
    using std::is_partitioned;
    using std::partition;
    using std::stable_partition;
    using std::partition_copy;
    using std::partition_point;

} // namespace ustl::algorithms

#endif // #ifndef USTL_ALGORITHMS_PARTITION_HPP