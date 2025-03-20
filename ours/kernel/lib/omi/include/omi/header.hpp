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
#ifndef OMI_OMI_HEADER_HPP
#define OMI_OMI_HEADER_HPP 1

#include <ours/types.hpp>

namespace omi {
    struct Header
    {
        u32 type;

        // Size of the payload immediately following this header. This
        // does not include the header itself nor any alignment padding
        // after the payload.
        u32 length;

        // Flags for this item.
        u32 flags;

        // Must be OMI_HEADER_MAGIC.
        u32 magic;

        // Must be the CRC32 of payload if ZBI_FLAGS_CRC32 is set,
        // otherwise must be ZBI_ITEM_NO_CRC32.
        u32 check_code;

        // Enable if flags was set HeaderFlags::Compressed.
        u32 extra;

        usize extra_1;
    };

    struct Kernel
    {
        usize entry_point;
        usize reserved_size;
    };

    struct OmiKernelHeader
    {
        Header  file;
        Header  main;
        Kernel  kernel;
    };

} // namespace omi

#endif // #ifndef OMI_OMI_HEADER_HPP