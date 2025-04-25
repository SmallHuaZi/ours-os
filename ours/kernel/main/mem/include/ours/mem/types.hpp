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
#include <ustl/util/type-list.hpp>

#include <arch/paging/mmu_flags.hpp>
#include <arch/paging/controls.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    using arch::paging::MmuFlags;
    using arch::paging::MapControl;
    using arch::paging::UnmapControl;
    using arch::paging::HarvestControl;

    enum class ZoneType {
        Dma OURS_IF_NOT_CFG(ZONE_DMA, = -1),
        Dma32 OURS_IF_NOT_CFG(ZONE_DMA32, = Dma),
        Normal,
        MaxNumZoneType,
    };
    typedef ustl::traits::UnderlyingTypeT<ZoneType> ZoneTypeVal;
    static_assert(usize(ZoneType::MaxNumZoneType) == NR_ZONES_PER_NODE, "");

    template <ZoneType>
    struct ZoneTypeMatcher;

    template <ZoneType Type, usize Pfn, bool Enable = true>
    using MaxZonePfn = ustl::TypeList<mem::ZoneTypeMatcher<Type>,
                                     ustl::traits::IntegralConstant<usize, Pfn>,
                                     ustl::traits::BoolConstant<Enable>>;
    
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
    typedef usize   Vpn;
    typedef usize   SecNum;

    FORCE_INLINE CXX11_CONSTEXPR 
    static auto pfn_to_phys(Pfn pfn) -> PhysAddr {  
        return PhysAddr(pfn << PAGE_SHIFT);  
    }

    template <typename T>
    FORCE_INLINE CXX11_CONSTEXPR 
    static auto phys_to_pfn(T phys_addr) -> Pfn {  
        static_assert(sizeof(T) == sizeof(PhysAddr));
        return Pfn(phys_addr >> PAGE_SHIFT);  
    }

    template <typename Addr>
    FORCE_INLINE
    auto resolve_page_range(Addr base, usize size) -> gktl::Range<usize> {
        static_assert(sizeof(Addr) == sizeof(VirtAddr));
        return {
            reinterpret_cast<VirtAddr>(base) >> PAGE_SHIFT,
            (size + PAGE_SIZE) >> PAGE_SHIFT
        };
    }

    FORCE_INLINE
    auto resolve_byte_range(PgOff pgoff, usize nr_pages) -> gktl::Range<usize> {
        return {
            pgoff << PAGE_SHIFT,
            nr_pages << PAGE_SHIFT,
        };
    }

    /// Forward declarations.
    /// Main classes for physical memory management.
    class PmFrame;
    class PmSection;
    class PmFolio;
    class PmZone;
    class PmNode;
    class ObjectCache;

    /// Auxiliary classes for physical memory management.
    class FrameQueue;

    /// Main classes for virtual memory management.
    class ArchVmAspace;
    class VmAspace;
    class VmArea;
    class VmPage;
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
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::ObjectCache);

OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmArea);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmAspace);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObject);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObjectPaged);
OURS_IMPL_MARKER_FOR(KernelObject, ours::mem::VmObjectPhysical);

#endif // #ifndef OURS_MEM_TYPES_HPP