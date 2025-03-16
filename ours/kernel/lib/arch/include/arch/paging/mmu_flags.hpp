// arch/paging ARCH/PAGING_MMU_FLAGS_HPP
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
#ifndef ARCH_PAGING_MMU_FLAGS_HPP
#define ARCH_PAGING_MMU_FLAGS_HPP 1

#include <ours/types.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/util/enum_bits.hpp>

namespace arch::paging {
    enum class MmuFlags: usize {
        Writable    = BIT(0),
        Readable    = BIT(1),
        Executable  = BIT(2),
        PermMask    = Writable | Readable | Executable,

        Cached      = BIT(3),
        User        = BIT(4),
    }; // enum class MmuFlags: usize
    USTL_ENABLE_ENUM_BITS(MmuFlags);

    template <typename ArchMmuFlags>
    auto down_cast(MmuFlags flags) -> ArchMmuFlags;

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_MMU_FLAGS_HPP