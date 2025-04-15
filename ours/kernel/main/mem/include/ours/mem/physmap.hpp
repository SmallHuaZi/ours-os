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
#ifndef OURS_MEM_PHYSMAP_HPP
#define OURS_MEM_PHYSMAP_HPP 1

#include <ours/mem/cfg.hpp>
#include <ours/mem/types.hpp>
#include <ours/arch/aspace_layout.hpp>

namespace ours::mem {
    /// The `PhysMap` represents a region of the kernel where all of useful physical memory
    /// is mapped in one large chunk. It's up to the individual architecture to decide
    /// how much to map but it's usually a fairly large chunk at the base of the kernel
    /// address space.
    struct PhysMap {
        CXX11_CONSTEXPR
        static PhysAddr const kPhysBase = ARCH_PHYSMAP_PHYS_BASE;

        CXX11_CONSTEXPR
        static VirtAddr const kVirtBase = ARCH_PHYSMAP_VIRT_BASE;
        static_assert(!(kVirtBase & ((MAX_PAGE_SIZE) - 1)), "");

        CXX11_CONSTEXPR
        static usize const kSize = ARCH_PHYSMAP_SIZE;

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto is_valid_phys_addr(T phys_addr) -> bool {
            static_assert(sizeof(usize) == sizeof(T));
            return kPhysBase <= reinterpret_cast<PhysAddr>(phys_addr) && 
                   reinterpret_cast<PhysAddr>(phys_addr) - kPhysBase < kSize;
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto is_valid_virt_addr(T virt_addr) -> bool {  
            static_assert(sizeof(usize) == sizeof(T));
            return kVirtBase <= reinterpret_cast<VirtAddr>(virt_addr) && 
                   reinterpret_cast<VirtAddr>(virt_addr) - kVirtBase < kSize; 
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto phys_to_virt(T phys_addr) -> VirtAddr {
            static_assert(sizeof(usize) == sizeof(T));
            if (PhysMap::is_valid_phys_addr(phys_addr)) {
                return VirtAddr(kVirtBase + (phys_addr - kPhysBase));
            }
            return VirtAddr();
        }

        template <typename U, typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto phys_to_virt(T phys_addr) -> U * {
            static_assert(sizeof(usize) == sizeof(T));
            return reinterpret_cast<U *>(PhysMap::phys_to_virt(phys_addr));
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto virt_to_phys(T virt_addr) -> PhysAddr {
            static_assert(sizeof(usize) == sizeof(T));
            if (PhysMap::is_valid_virt_addr(virt_addr)) {
                return PhysAddr(kPhysBase + (virt_addr - kVirtBase));
            }
            return PhysAddr();
        }

        template <typename U, typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto virt_to_phys(T virt_addr) -> U * {  
            static_assert(sizeof(usize) == sizeof(T));
            return reinterpret_cast<U *>(PhysMap::virt_to_phys(virt_addr)); 
        }
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PHYSMAP_HPP