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
#ifndef ARCH_PAGING_ARCH_MMU_FLAGS_HPP
#define ARCH_PAGING_ARCH_MMU_FLAGS_HPP 1

#include <arch/paging/mmu_flags.hpp>
#include <arch/macro/mmu.hpp>
#include <ustl/util/enum_bits.hpp>

namespace arch::paging {
    enum class ArchMmuFlags: usize {
        Present      = X86_MMUF_PRESENT,
        Writable     = X86_MMUF_WRITABLE,
        User         = X86_MMUF_USER,
        WriteThrough = X86_MMUF_WRITE_THROUGH,
        Discache     = X86_MMUF_DISCACHE,
        Accessed     = X86_MMUF_ACCESSED,
        Dirty        = X86_MMUF_DIRTY,
        PageSize     = X86_MMUF_PAGE_SIZE,
        Global       = X86_MMUF_GLOBAL,
        NoExcutable  = X86_MMUF_NOEXCUTABLE,
    };
    USTL_ENABLE_ENUM_BITS(ArchMmuFlags);

    template <>
    FORCE_INLINE CXX11_CONSTEXPR
    auto down_cast(MmuFlags const flags) -> ArchMmuFlags
    {
        ArchMmuFlags target{};
        if (bool(flags & MmuFlags::Present)) {
            target |= ArchMmuFlags::Present;
        }
        if (!bool(flags & MmuFlags::Executable)) {
            target |= ArchMmuFlags::NoExcutable;
        }
        if (bool(flags & MmuFlags::Writable)) {
            target |= ArchMmuFlags::Writable;
        }
        if (bool(flags & MmuFlags::User)) {
            target |= ArchMmuFlags::User;
        }
        if (bool(flags & MmuFlags::DisCache)) {
            target |= ArchMmuFlags::Discache;
        }

        return target;
    }

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_ARCH_MMU_FLAGS_HPP