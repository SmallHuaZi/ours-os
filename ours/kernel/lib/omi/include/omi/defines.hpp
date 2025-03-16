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

#ifndef OMI_DEFINES_HPP
#define OMI_DEFINES_HPP 1

#define OMI_SIZEOF_HEADER   32
#define OMI_HEADER_MAGIC   0x5352554F

#define OMI_HEADER_ALIGNMENT 8

#define OMIT_CONTAINER     0    // Container
#define OMIT_KERNEL     1       // Kernel image
#define OMIT_USBOOT     2       // User boot

#define OMIF_ZIPPED    1

#endif // #ifndef OMI_DEFINES_HPP