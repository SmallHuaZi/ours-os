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

#ifndef OURS_MEM_MMU_FLAGS_HPP
#define OURS_MEM_MMU_FLAGS_HPP 1

#include <ours/types.hpp>
#include <ours/marco_abi.hpp>

#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    enum class MmuFlags: usize {
        Writable    = BIT(0),
        Readable    = BIT(1),
        Executable  = BIT(2),
        PermissionsMask = Writable | Readable | Executable,

        Cached      = BIT(3),
        User        = BIT(4),
    }; // enum class MmuFlags: usize
    USTL_ENABLE_ENUM_BITS(MmuFlags);

    template <typename ArchMmuFlags>
    auto down_cast(MmuFlags flags) -> ArchMmuFlags;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MMU_FLAGS_HPP