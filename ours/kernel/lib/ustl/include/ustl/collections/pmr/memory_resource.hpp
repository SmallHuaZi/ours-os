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

#ifndef USTL_COLLECTIONS_PMR_MEMORY_RESOURCE_HPP
#define USTL_COLLECTIONS_PMR_MEMORY_RESOURCE_HPP 1

#include <ustl/config.hpp>
#include <boost/container/pmr/memory_resource.hpp>

namespace ustl::collections::pmr {
    struct MemoryResource
        : public boost::container::pmr::memory_resource
    {
        typedef boost::container::pmr::memory_resource  Base;

        virtual auto do_allocate(usize bytes, usize align) -> void *
        {  return 0;  }

        virtual auto do_deallocate(void *p, usize bytes, usize align) -> void
        {  return; }

        virtual auto do_is_equal(Base const &other) const USTL_NOEXCEPT -> bool
        {  return this == &other;  }
    };

} // namespace ustl::collections::pmr

#endif // #ifndef USTL_COLLECTIONS_PMR_MEMORY_RESOURCE_HPP