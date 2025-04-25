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
    /// Layout statifing the standard flag bits in a PTE 
    enum class X86MmuFlags: usize {
        Present      = X86_MMUF_PRESENT,
        Writable     = X86_MMUF_WRITABLE,
        User         = X86_MMUF_USER,
        WriteThrough = X86_MMUF_WRITE_THROUGH,
        Discache     = X86_MMUF_DISCACHE,
        Accessed     = X86_MMUF_ACCESSED,
        Dirty        = X86_MMUF_DIRTY,
        PageSize     = X86_MMUF_PAGE_SIZE,
        Global       = X86_MMUF_GLOBAL,
        NoExecutable = X86_MMUF_NOEXECUTABLE,

        PermMask     = Writable,
    };
    USTL_ENABLE_ENUM_BITMASK(X86MmuFlags);

    template <>
    FORCE_INLINE CXX11_CONSTEXPR
    auto mmuflags_cast(MmuFlags const flags) -> X86MmuFlags {
        X86MmuFlags target{};
        if (!!(flags & MmuFlags::Present)) {
            target |= X86MmuFlags::Present;
        }
        if (!(flags & MmuFlags::Executable)) {
            target |= X86MmuFlags::NoExecutable;
        }
        if (!!(flags & MmuFlags::Writable)) {
            target |= X86MmuFlags::Writable;
        }
        if (!!(flags & MmuFlags::User)) {
            target |= X86MmuFlags::User;
        }
        if (!!(flags & MmuFlags::Discache)) {
            target |= X86MmuFlags::Discache;
        }

        return target;
    }

    template <>
    FORCE_INLINE CXX11_CONSTEXPR
    auto mmuflags_cast(X86MmuFlags const flags) -> MmuFlags {
        auto target = MmuFlags::Readable;
        if (!!(flags & X86MmuFlags::Present)) {
            target |= MmuFlags::Present;
        }
        if (!(flags & X86MmuFlags::NoExecutable)) {
            target |= MmuFlags::Executable;
        }
        if (!!(flags & X86MmuFlags::Writable)) {
            target |= MmuFlags::Writable;
        }
        if (!!(flags & X86MmuFlags::User)) {
            target |= MmuFlags::User;
        }
        if (!!(flags & X86MmuFlags::Discache)) {
            target |= MmuFlags::Discache;
        }

        return target;
    }

    typedef X86MmuFlags     ArchMmuFlags;

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_ARCH_MMU_FLAGS_HPP