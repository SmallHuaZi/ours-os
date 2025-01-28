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

#ifndef USTL_COLLECTIONS_PMR_MONOTONIC_MEMORY_RESOURCE_HPP
#define USTL_COLLECTIONS_PMR_monotonic_MEMORY_RESOURCE_HPP 1

#include <boost/container/pmr/monotonic_buffer_resource.hpp>

namespace ustl::collections::pmr {
    using MonotonicBufferResource = boost::container::pmr::monotonic_buffer_resource;

} // namespace ustl::collections::pmr

#endif // #ifndef USTL_COLLECTIONS_PMR_MONOTONIC_MEMORY_RESOURCE_HPP