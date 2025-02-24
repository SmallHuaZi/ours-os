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
        static PhysAddr const PHYS_BASE = ARCH_PHYSMAP_PHYS_BASE;

        CXX11_CONSTEXPR
        static VirtAddr const VIRT_BASE = ARCH_PHYSMAP_VIRT_BASE;
        static_assert(!(VIRT_BASE & ((MAX_PAGE_SIZE) - 1)), "");

        CXX11_CONSTEXPR
        static usize const SIZE = ARCH_PHYSMAP_SIZE;

        CXX11_CONSTEXPR
        static auto is_valid_phys_addr(PhysAddr phys_addr) -> bool
        {  return PHYS_BASE <= phys_addr && phys_addr - PHYS_BASE < SIZE; }

        CXX11_CONSTEXPR
        static auto is_valid_virt_addr(VirtAddr virt_addr) -> bool
        {  return VIRT_BASE <= virt_addr && virt_addr - VIRT_BASE < SIZE; }

        CXX11_CONSTEXPR
        static auto phys_to_virt(PhysAddr phys_addr) -> VirtAddr
        {  
            if (PhysMap::is_valid_phys_addr(phys_addr)) {
                return VirtAddr(VIRT_BASE + (phys_addr - PHYS_BASE));
            }
            return VirtAddr();
        }

        template <typename T>
        CXX11_CONSTEXPR
        static auto phys_to_virt(PhysAddr phys_addr) -> T * 
        {  return reinterpret_cast<T *>(PhysMap::phys_to_virt(phys_addr)); }

        template <typename T>
        CXX11_CONSTEXPR
        static auto phys_to_virt(T *phys_addr) -> VirtAddr 
        {  return PhysMap::phys_to_virt(reinterpret_cast<PhysAddr>(phys_addr)); }

        CXX11_CONSTEXPR
        static auto virt_to_phys(VirtAddr virt_addr) -> PhysAddr
        {  
            if (PhysMap::is_valid_virt_addr(virt_addr)) {
                return PhysAddr(PHYS_BASE + (virt_addr - VIRT_BASE));
            }
            return PhysAddr();
        }

        template <typename T>
        CXX11_CONSTEXPR
        static auto virt_to_phys(VirtAddr virt_addr) -> T * 
        {  return reinterpret_cast<T *>(PhysMap::virt_to_phys(virt_addr)); }

        template <typename T>
        CXX11_CONSTEXPR
        static auto virt_to_phys(T *virt_addr) -> VirtAddr 
        {  return PhysMap::virt_to_phys(reinterpret_cast<PhysAddr>(virt_addr)); }
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PHYSMAP_HPP