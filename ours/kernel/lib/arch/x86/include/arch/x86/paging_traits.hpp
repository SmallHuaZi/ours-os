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

#ifndef ARCH_X86_PAGING_CONFIG_HPP
#define ARCH_X86_PAGING_CONFIG_HPP 1

#include <arch/types.hpp>
#include <arch/x86/mmu.hpp>

#define X86_ENABLE_PAE 1

namespace arch::x86 {
    template <usize Level>
    struct PagingTraits;

    template <>
    struct PagingTraits<2>
    {
        CXX11_CONSTEXPR 
        static auto const PAGE_LEVEL = 2;

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(usize level, VirtAddr virt) -> usize
        {
            CXX11_CONSTEXPR 
            static auto const INDEX_BITS = 10;

            CXX11_CONSTEXPR 
            static auto const INDEX_MASK = BIT(INDEX_BITS) - 1;

            CXX11_CONSTEXPR 
            static usize const MAP[][2] { 
                {0, 0}, 
                {X86_PT_SHIFT, INDEX_MASK},
                {X86_PDP_SHIFT, INDEX_MASK},
            };

            return (virt >> MAP[level][0]) & MAP[level][1];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const PAGE_SIZE_MAP[] = 
            { KB(4), MB(4), GB(4) };

            return PAGE_SIZE_MAP[level];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entry(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const MAP[]
            { 0, 1024, 1024 };

            return MAP[level];
        }
    };

    template <>
    struct PagingTraits<3>
    {
        CXX11_CONSTEXPR 
        static auto const PAGE_LEVEL = 3;

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(usize level, VirtAddr virt) -> usize
        {
            CXX11_CONSTEXPR 
            static auto const INDEX_BITS = 9;

            CXX11_CONSTEXPR 
            static auto const INDEX_MASK = BIT(INDEX_BITS) - 1;

            CXX11_CONSTEXPR 
            static usize const MAP[][2] { 
                {0, 0}, 
                {X86_PT_SHIFT, INDEX_MASK},
                {X86_PDP_SHIFT, INDEX_MASK},
                {X86_PDP_SHIFT, BIT(2) - 1},
            };

            return (virt >> MAP[level][0]) & MAP[level][1];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const PAGE_SIZE_MAP[] = 
            { KB(4), MB(2), GB(1), GB(4) };

            return PAGE_SIZE_MAP[level];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entry(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const MAP[]
            { 0, 512, 512, 4 };

            return MAP[level];
        }
    };

    template <>
    struct PagingTraits<4> 
    {
        CXX11_CONSTEXPR 
        static auto const PAGE_LEVEL = 4;

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(usize level, VirtAddr virt) -> usize
        {
            CXX11_CONSTEXPR 
            static usize const SHIFT[] {
                0,
                X86_PT_SHIFT,
                X86_PD_SHIFT,
                X86_PDP_SHIFT,
                X86_PML4_SHIFT,
            };

            return (virt >> SHIFT[level]) & 0x1FF;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const PAGE_SIZE_MAP[]
            { KB(4), MB(2), GB(1), GB(512), TB(256) };

            return PAGE_SIZE_MAP[level];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entry(usize) -> usize
        {  return 512;  }
    };
    
    template <>
    struct PagingTraits<5> 
    {
        CXX11_CONSTEXPR static auto const PAGE_LEVEL = 5;

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(usize level, VirtAddr virt) -> usize
        {
            CXX11_CONSTEXPR 
            static usize const SHIFT[] {
                0,
                X86_PT_SHIFT,
                X86_PD_SHIFT,
                X86_PDP_SHIFT,
                X86_PML4_SHIFT,
                X86_PML5_SHIFT
            };

            return (virt >> SHIFT[level]) & 0x1FF;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(usize level) -> usize
        {
            CXX11_CONSTEXPR
            static usize const PAGE_SIZE_MAP[]
            { KB(4), MB(2), GB(1), GB(512), TB(256), PB(128) };

            return PAGE_SIZE_MAP[level];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entry(usize) -> usize
        {  return 512;  }
    };

} // namespace arch::x86 

#endif // #ifndef ARCH_X86_PAGING_CONFIG_HPP