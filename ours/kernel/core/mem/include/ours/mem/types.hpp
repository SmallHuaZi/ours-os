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

#ifndef OURS_MEM_TYPES_HPP
#define OURS_MEM_TYPES_HPP 1

#include <ours/marker.hpp>
#include <ours/mem/constant.hpp>

#include <ustl/views/span.hpp>

namespace ours::mem {
    enum ZoneType {
        Dma OURS_IF_NOT_CFG(ZONE_DMA, = -1),
        Dma32 OURS_IF_NOT_CFG(ZONE_DMA32, = Dma),
        Normal,
        MaxNumZoneType,
    };
    static_assert(MaxNumZoneType < NR_ZONES_PER_NODE, "");

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto to_string(ZoneType type) -> char const *
    {
        switch (type) {
            case ZoneType::Dma:     return "Dma";
            case ZoneType::Dma32:   return "Dma32";
            case ZoneType::Normal:  return "Normal";
            default: return "Anonymous type";
        }
    }

    typedef usize   Pfn;
    typedef isize   NodeId;

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto pfn_to_phys(Pfn pfn) -> PhysAddr
    {  return PhysAddr(pfn << FRAME_SHIFT);  }

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto phys_to_pfn(PhysAddr phys_addr) -> Pfn
    {  return Pfn(phys_addr >> FRAME_SHIFT);  }

    /// Forward declarations.
    /// Main classes for physical memory management.
    class PmFrame;
    class PmFolio;
    class PmZone;
    class PmNode;

    /// Auxiliary classes for physical memory management.
    class FrameQueue;

    /// Main classes for virtual memory management.
    class ArchVmAspace;
    class VmAspace;
    class VmArea;
    class VmRootArea;
    class VmObject;

} // namespace ours::mem

/// Mark types above as kernel object to enable particular static analysis.
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmFolio);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmFrame);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmZone);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmNode);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmArea);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmAspace);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObject);

#endif // #ifndef OURS_MEM_TYPES_HPP