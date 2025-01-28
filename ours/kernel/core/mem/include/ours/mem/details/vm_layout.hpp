// ours/mem/details OURS/MEM/DETAILS_VM_LAYOUT_HPP
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

#ifndef OURS_MEM_DETAILS_VM_LAYOUT_HPP
#define OURS_MEM_DETAILS_VM_LAYOUT_HPP 1

namespace ours::mem::details {
    extern "C" {
        /// The start and end virtual address of the complete kernel image.
        extern char KERNEL_START[];
        extern char KERNEL_END[];

        /// Without the content about boot of system 
        extern char KERNEL_CORE_START[];
        extern char KERNEL_CORE_END[];

        extern char KERNEL_CODE_START[];
        extern char KERNEL_CODE_END[];

        extern char KERNEL_DATA_START[];
        extern char KERNEL_DATA_END[];

        extern char KERNEL_RODATA_START[];
        extern char KERNEL_RODATA_END[];

        extern char KERNEL_BSS_START[];
        extern char KERNEL_BSS_END[];
    }

} // namespace ours::mem::details

#endif // #ifndef OURS_MEM_DETAILS_VM_LAYOUT_HPP