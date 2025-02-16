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

#ifndef OURS_ARCH_X86_MMU_PAGING_CONFIG_HPP
#define OURS_ARCH_X86_MMU_PAGING_CONFIG_HPP 1

#include <ours/mem/types.hpp>
#include <ours/marco_abi.hpp>

#include <ustl/util/enum_bits.hpp>

#ifndef OURS_CONFIG_PAGING_LEVEL
#define OURS_CONFIG_PAGING_LEVEL    5
#endif // #ifndef OURS_CONFIG_PAGING_LEVEL

#ifndef OURS_CONFIG_PAGE_SIZE
#define OURS_CONFIG_PAGE_SIZE     4K
#endif // #ifndef OURS_CONFIG_PAGING_SHIFT

#define OURS_PAGE_SIZE    OURS_MPL_CONTACT(Size, OURS_CONFIG_PAGE_SIZE)

namespace ours::mem {
    enum class X86MmuFlags: u64 {
        /// Indicates whether the page is present in memory. 
        /// If this flag is cleared, a page fault will occur 
        /// if the page is accessed.
        Present       = BIT(0),

        /// Determines whether the page is readable and writable. 
        /// If set, the page can be written to; otherwise, it is 
        /// read-only.
        Writable      = BIT(1),

        /// Indicates whether the page can be accessed in user mode 
        /// or only in supervisor (kernel) mode. If set, the page can 
        /// be accessed by user-mode code; if cleared, only supervisor-mode 
        /// code can access it.
        User          = BIT(2),

        /// Controls the cacheability of the page. If set, write-through 
        /// caching is enabled for this page, which means writes to this 
        /// page are immediately visible to other processors or devices.
        WriteThrough  = BIT(3),

        /// If set, caching is disabled for this page. If cleared, caching 
        /// is enabled.
        Discache      = BIT(4),

        /// This flag is set by the MMU to indicate whether the page has been 
        /// accessed since the last time the flag was cleared.
        Accessed      = BIT(5),

        /// This flag is set by the MMU to indicate whether the page has been 
        /// written to (i.e., dirty).
        Dirty         = BIT(6),

        /// On x86-64, this flag indicates whether the page entry corresponds 
        /// to a 4KB page (if cleared) or a 4MB page (if set). It’s typically 
        /// used in large page table entries.
        PAGE_SIZE  = BIT(7),

        /// If set, the page is marked as global. This means the page is not 
        /// invalidated on a context switch, which can optimize performance in 
        /// some cases (e.g., for kernel mappings).
        Global    = BIT(8),

        /// This flag controls the execution of code in the page. If set, the page 
        /// is marked as non-executable, which helps prevent code execution from data 
        /// sections and mitigates certain types of attacks like buffer overflows.
        NonExecutable = BIT(63),

        Mask      = 0,
    }; // enum class X86MmuFlags
    USTL_ENABLE_ENUM_BITS(X86MmuFlags);

    enum class PageSizeFlags {
	    Size4K,
	    Size2M,
	    Size1G,
	    Size512G,
    };

    struct CommonPagingConfig
    {
        CXX11_CONSTEXPR static u64 const PAGE_SHIFT = {
            PageSizeFlags::OURS_PAGE_SIZE == PageSizeFlags::Size2M ? 2 :
            PageSizeFlags::OURS_PAGE_SIZE == PageSizeFlags::Size1G ? 30 :
            PageSizeFlags::OURS_PAGE_SIZE == PageSizeFlags::Size512G ? 35 : 12
        };

        CXX11_CONSTEXPR static u64 const PAGE_SIZE = (1 << PAGE_SHIFT);
        CXX11_CONSTEXPR static u64 const PAGE_MASK = ~(PAGE_SIZE - 1);

        typedef X86MmuFlags     ArchMmuFlags;
    };

    struct PagingVia4Level
        : CommonPagingConfig
    {
        CXX11_CONSTEXPR static auto const PAGE_LEVEL = 4;
        CXX11_CONSTEXPR static auto const NR_PTE = 512;

        CXX11_CONSTEXPR static auto const PGD_SHIFT = 39;
        CXX11_CONSTEXPR static auto const PGD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PUD_SHIFT = 21;
        CXX11_CONSTEXPR static auto const PUD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PLD_SHIFT = 12;
        CXX11_CONSTEXPR static auto const PLD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PTE_SHIFT = 12;
        CXX11_CONSTEXPR static auto const PTE_MASK = 0x1FF;

        static auto index_of(VirtAddr adr, usize level) -> usize 
        {
            CXX11_CONSTEXPR usize const map[][2] = {
                {  0, 0},
                {  PTE_SHIFT, PTE_MASK },
                {  PLD_SHIFT, PLD_MASK },
                {  PUD_SHIFT, PUD_MASK },
                {  PGD_SHIFT, PGD_MASK },
            };

            return (adr >> map[level][0]) & map[level][1];
        }

        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR usize const map[] = {
                PAGE_SHIFT, 
                PAGE_SHIFT, 
                PAGE_SHIFT, 
                PAGE_SHIFT, 
            };

            return map[level];
        }
    };
    
    struct PagingVia5Level
        : CommonPagingConfig
    {
        CXX11_CONSTEXPR static auto const PAGE_LEVEL = 5;
        CXX11_CONSTEXPR static auto const NR_PTE = 512;

        CXX11_CONSTEXPR static auto const PGD_SHIFT = 47;
        CXX11_CONSTEXPR static auto const PGD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PUD_SHIFT = 39;
        CXX11_CONSTEXPR static auto const PUD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PMD_SHIFT = 30;
        CXX11_CONSTEXPR static auto const PMD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PLD_SHIFT = 21;
        CXX11_CONSTEXPR static auto const PLD_MASK = 0x1FF;
        CXX11_CONSTEXPR static auto const PTE_SHIFT = 12;
        CXX11_CONSTEXPR static auto const PTE_MASK = 0x1FF;

        static auto index_of(VirtAddr adr, usize level) -> usize 
        {
            CXX11_CONSTEXPR usize const map[][2] = {
                {  0, 0},
                {  PTE_SHIFT, PTE_MASK },
                {  PLD_SHIFT, PLD_MASK },
                {  PMD_SHIFT, PMD_MASK },
                {  PUD_SHIFT, PUD_MASK },
                {  PGD_SHIFT, PGD_MASK },
            };

            return (adr >> map[level][0]) & map[level][1];
        }

        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR usize const map[] = {
                PAGE_SHIFT,
                PAGE_SHIFT,
                PAGE_SHIFT,
                PAGE_SHIFT,
                PAGE_SHIFT,
            };

            return map[level];
        }
    };

#if OURS_CONFIG_PAGING_LEVEL == 5
    using X86PagingConfig = PagingVia5Level;
#elif OURS_CONFIG_PAGING_LEVEL == 4
    using X86PagingConfig = PagingVia4Level;
#endif // #if OURS_CONFIG_PAGING_LEVEL

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_X86_MMU_PAGING_CONFIG_HPP