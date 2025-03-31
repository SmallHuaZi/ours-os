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

#include <ours/init.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/cfg.hpp>

#include <ours/phys/handoff.hpp>

#include <ustl/mem/align.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/algorithms/minmax.hpp>

#include <gktl/range.hpp>

#ifdef OURS_CONFIG_MEM_HOTPLUG
#define MEM_INIT_CODE
#define MEM_INIT_DATA
#else
#define MEM_INIT_CODE  INIT_CODE
#define MEM_INIT_DATA  INIT_DATA
#endif

namespace ours::mem {
    struct EarlyMem {
        typedef phys::BootMem::IterationContext     IterationContext;

        static auto init(phys::MemoryHandoff &handoff) -> void;

        static auto protect(PhysAddr base, usize size) -> void
        {  g_bootmem->protect(base, size);  }

        template <typename T>
        static auto allocate(usize n, NodeId nid) -> T *
        {  return allocate<T>(n, alignof(T), nid);  }

        template <typename T>
        static auto allocate(usize n, usize align, NodeId nid) -> T *;

        template <typename T>
        static auto allocate(usize n, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> T *;

        template <typename T>
        static auto deallocate(T *ptr, usize size) -> void;

        static auto iterate(IterationContext &context) -> ustl::Option<bootmem::Region>
        {  return g_bootmem->iterate(context);  }

        static auto get_node_pfn_range(NodeId nid) -> gktl::Range<Pfn>;

        static auto count_present_frames(Pfn start, Pfn end) -> usize;

        static auto start_address() -> PhysAddr
        {  return g_bootmem->start_address;  }

        INIT_DATA
        static inline phys::EarlyMem *g_bootmem;
    };

    template <typename T>
    inline auto EarlyMem::allocate(usize n, usize align, NodeId nid) -> T *
    {
        DEBUG_ASSERT(g_bootmem);
        PhysAddr phys_addr = g_bootmem->allocate(sizeof(T) * n, align, g_bootmem->start_address, g_bootmem->end_address, nid);
        return PhysMap::phys_to_virt<T>(phys_addr);
    }

    template <typename T>
    inline auto EarlyMem::allocate(usize n, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> T * 
    {
        DEBUG_ASSERT(g_bootmem);
        PhysAddr phys_addr = g_bootmem->allocate(sizeof(T) * n, align, start, end, nid);
        return PhysMap::phys_to_virt<T>(phys_addr);
    }

    template <typename T>
    inline auto EarlyMem::deallocate(T *ptr, usize n) -> void 
    {
        DEBUG_ASSERT(g_bootmem);
        PhysAddr phys_addr = PhysMap::virt_to_phys(ptr);
        g_bootmem->deallocate(ptr, sizeof(T) * n);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_EARLY_HPP