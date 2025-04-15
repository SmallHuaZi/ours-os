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

#include <ours/panic.hpp>
#include <ours/types.hpp>
#include <ours/marker.hpp>
#include <ours/mem/cfg.hpp>

#include <ustl/views/span.hpp>
#include <ustl/util/type-map.hpp>
#include <arch/paging/mmu_flags.hpp>

namespace ours::mem {
    using arch::paging::MmuFlags;

    enum ZoneType {
        Dma OURS_IF_NOT_CFG(ZONE_DMA, = -1),
        Dma32 OURS_IF_NOT_CFG(ZONE_DMA32, = Dma),
        Normal,
        MaxNumZoneType,
    };
    static_assert(MaxNumZoneType == NR_ZONES_PER_NODE, "");

    template <ZoneType>
    struct ZoneTypeMatcher;

    template <ZoneType Type, usize Pfn>
    using MaxZonePfn = ustl::TypeMapEntry<mem::ZoneTypeMatcher<Type>, ustl::traits::IntegralConstant<usize, Pfn>>;

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto to_string(ZoneType type) -> char const * {
        switch (type) {
            case ZoneType::Dma:     return "Dma";
            case ZoneType::Dma32:   return "Dma32";
            case ZoneType::Normal:  return "Normal";
        }
        OX_PANIC("Anonymous zone type is not allowed.");
    }

    /// `SecNum` is a shordhand of section table entry
    /// 
    /// A address has been seen like the following layout:
    ///     | Top section index | ... | leaf section index | Control flags |
    typedef usize   Pfn;
    typedef usize   SecNum;
    typedef isize   NodeId;

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto pfn_to_phys(Pfn pfn) -> PhysAddr {  
        return PhysAddr(pfn << PAGE_SHIFT);  
    }

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto phys_to_pfn(PhysAddr phys_addr) -> Pfn {  
        return Pfn(phys_addr >> PAGE_SHIFT);  
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto phys_to_secnum(PhysAddr phys_addr) -> SecNum {  
        return phys_addr >> SECTION_SHIFT;  
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto secnum_to_phys(SecNum secnum) -> PhysAddr {  
        return secnum << SECTION_SHIFT;  
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto pfn_to_secnum(Pfn pfn) -> SecNum {  
        return pfn >> PFN_SECTION_SHIFT;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto secnum_to_pfn(SecNum secnum) -> Pfn {  
        return secnum << PFN_SECTION_SHIFT;
    }

    /// A pre-allocated storage reserve designed for scenarios requiring specialized physical page management. 
    /// It serves two core purposes:
    /// 
    /// 1. **Bypass Default Allocators**
    ///    Provides dedicated storage when the standard `alloc_frame`, `free_frame` interfaces are unsuitable 
    ///    (e.g., `PMM` is not unuseful).
    /// 
    /// 2. **Reuse Idle Resources**
    ///    Actively reclaims and repurposes underutilized `PmFrame` objects to eliminate redundant allocation 
    ///    cycles.
    struct Altmap {
    	Pfn start;
    	Pfn end;

        // free pages set aside in the mapping for memmap storage
    	Pfn reserve;

        // pages reserved to meet allocation alignments
    	usize free;
    	usize align;

        // track pages consumed, public to caller 
    	usize alloc;
    };

    /// Forward declarations.
    /// Main classes for physical memory management.
    class PmFrame;
    class PmSection;
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
    class VmObjectPaged;
    class VmObjectPhysical;
} // namespace ours::mem

/// Mark types above as kernel object to enable particular static analysis.
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmSection);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmFolio);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmFrame);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmZone);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::PmNode);

OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmArea);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmAspace);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObject);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObjectPaged);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObjectPhysical);

#endif // #ifndef OURS_MEM_TYPES_HPP