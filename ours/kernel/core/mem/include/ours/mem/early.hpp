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

#ifndef OURS_MEM_EARLY_HPP
#define OURS_MEM_EARLY_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/physmap.hpp>

#include <ustl/function/fn.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    struct EarlyMem {
        static auto add_range(PhysAddr base, usize size) -> void
        {  return add_range(base, size, NodeId{0});  }

        static auto add_range(PhysAddr base, usize size, NodeId id) -> void;

        static auto remove_range(PhysAddr base, usize size) -> void;

        static auto reserve_range(PhysAddr base, usize size) -> void;

        template <typename T>
        static auto allocate(usize align) -> T *;

        static auto allocate(usize size, usize align) -> PhysAddr;

        static auto allocate_bounded(usize size, usize align, gktl::Range<PhysAddr> bound) -> PhysAddr;

        static auto deallocate(usize size, usize align) -> PhysAddr;

        static auto for_each_all_regions(ustl::function::Fn<auto (MemRegion const &) -> void>) -> void;
    };

    template <typename T>
    inline auto EarlyMem::allocate(usize align) -> T *
    {
        PhysAddr phys_addr = EarlyMem::allocate(sizeof(T), align);
        return PhysMap::phys_to_virt<T>(phys_addr);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_EARLY_HPP