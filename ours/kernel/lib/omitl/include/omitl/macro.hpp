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
#ifndef OMI_MACRO_HPP
#define OMI_MACRO_HPP 1

#define OMI_SIZEOF_HEADER  24 
#define OMI_HEADER_MAGIC   0x5352554F

#define OMI_ALIGNMENT 8

// Types
#define OMIT_CONTAINER      0x746E6F63   // "cont"
#define OMIT_KPACKAGE       0x676b706b   // "kpkg"
#define OMIT_RAMDISK        0x444D4152   // "RAMD"
#define OMIT_MEMORY         0x444D4152   // "RAMD"

/// Flags
#define OMIF_ZIPPED       1    // Zipped image
#define OMIF_FIXED_ADDR   2    // Expectation to load at fixed address.
#define OMIF_CRC32        4    // Expectation to load at fixed address.

/// Kernel signature
#define OMI_RAW_EXE_SIGNATURE 0x00444D57   // "RAW"
#define OMI_ELF_EXE_SIGNATURE 0x00666c65   // "elf"

#endif // #ifndef OMI_MACRO_HPP