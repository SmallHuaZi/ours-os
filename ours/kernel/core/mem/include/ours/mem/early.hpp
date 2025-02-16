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
#include <ours/mem/constant.hpp>

#include <ustl/mem/align.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/algorithms/minmax.hpp>

#ifdef OURS_CONFIG_MEM_HOTPLUG
#define MEM_INIT_CODE
#define MEM_INIT_DATA
#else
#define MEM_INIT_CODE  INIT_CODE
#define MEM_INIT_DATA  INIT_DATA
#endif

namespace ours::mem {
    struct EarlyMem {
        static auto init() -> void;

        static auto add_range(PhysAddr base, usize size) -> void
        {  return add_range(base, size, NodeId{0});  }

        static auto add_range(PhysAddr base, usize size, NodeId id) -> void;

        static auto remove_range(PhysAddr base, usize size) -> void;

        static auto protect_range(PhysAddr base, usize size) -> void;

        static auto allocate(usize size, usize align) -> PhysAddr;

        static auto allocate(usize size, usize align, NodeId nid) -> PhysAddr;

        template <typename T>
        static auto allocate(NodeId nid, usize n, usize align = alignof(T)) -> T *;

        template <typename T>
        static auto allocate(usize n, usize align = alignof(T)) -> T *;

        static auto allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end) -> PhysAddr;

        static auto deallocate(PhysAddr ptr, usize align) -> void;

        using RegionHandler = ustl::function::Fn<auto (usize base, usize size, NodeId nid) -> void>;
        static auto for_each_all_regions(RegionHandler const &) -> void;

        using NodeRegionHandler = ustl::function::Fn<auto (Pfn start, Pfn end) -> void>;
        static auto for_each_node_regions(NodeId nid, NodeRegionHandler const &) -> void;

        using NodePfnRangeHandler = ustl::function::Fn<auto (Pfn start, Pfn end) -> void>;
        static auto for_each_node_pfn_range(NodeId nid, NodePfnRangeHandler const &) -> void;

        static auto count_absent_frames(Pfn start, Pfn end) -> usize;

        static auto start_address() -> PhysAddr;
    };

    template <typename T>
    inline auto EarlyMem::allocate(usize n, usize align) -> T *
    {
        PhysAddr phys_addr = EarlyMem::allocate(sizeof(T) * n, align);
        return PhysMap::phys_to_virt<T>(phys_addr);
    }

    inline auto EarlyMem::count_absent_frames(Pfn start, Pfn end) -> usize
    {
        usize nr_absent = end - start;
        EarlyMem::RegionHandler const handler = [&] (usize base, usize size, NodeId noid) {
            Pfn rp_start = phys_to_pfn(ustl::mem::align_up(base, FRAME_SIZE));
            Pfn rp_end = phys_to_pfn(ustl::mem::align_down(base + size, FRAME_SIZE));

            rp_start = ustl::algorithms::clamp(rp_start, start, end);
            rp_end = ustl::algorithms::clamp(rp_end, start, end);

            nr_absent -= rp_end - rp_start;
        };
        EarlyMem::for_each_all_regions(handler);
        return nr_absent;
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_EARLY_HPP