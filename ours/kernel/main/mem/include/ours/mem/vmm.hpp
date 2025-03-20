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

#ifndef OURS_MEM_VMM_HPP
#define OURS_MEM_VMM_HPP 1

#include <ours/mem/types.hpp>

namespace ours::mem {
    /// Do not use the following symbols directly.
    extern char const KERNEL_IMAGE_START[] LINK_NAME("__executable_start");
    extern char const KERNEL_IMAGE_END[] LINK_NAME("__executable_end");

    extern char const KERNEL_CODE_START[] LINK_NAME("__code_start");
    extern char const KERNEL_CODE_END[] LINK_NAME("__code_end");

    extern char const KERNEL_DATA_START[] LINK_NAME("__data_start");
    extern char const KERNEL_DATA_END[] LINK_NAME("__data_end");

    extern char const KERNEL_RODATA_START[] LINK_NAME("__rodata_start");
    extern char const KERNEL_RODATA_END[] LINK_NAME("__rodata_end");

    extern char const KERNEL_INIT_START[] LINK_NAME("__init_start");
    extern char const KERNEL_INIT_END[] LINK_NAME("__init_end");

    extern char const KERNEL_BSS_START[] LINK_NAME("__bss_start");
    extern char const KERNEL_BSS_END[] LINK_NAME("__bss_end");

    NO_MANGLE PhysAddr g_kernel_phys_base;
    NO_MANGLE VirtAddr g_kernel_virt_base;

    FORCE_INLINE
    auto get_kernel_phys_base() -> PhysAddr { 
        return g_kernel_phys_base;  
    }

    FORCE_INLINE
    auto get_kernel_virt_base() -> VirtAddr { 
        return g_kernel_virt_base;  
    }

    FORCE_INLINE
    auto get_kernel_size() -> usize
    { return  KERNEL_IMAGE_END - KERNEL_IMAGE_START; }

    auto is_kernel_address() -> bool;

    auto virt_to_phys(VirtAddr) -> PhysAddr;

    auto phys_to_virt(PhysAddr) -> VirtAddr;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VMM_HPP