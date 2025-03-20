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
#ifndef ARCH_PAGING_X86_PAGINGS_HPP
#define ARCH_PAGING_X86_PAGINGS_HPP 1

#include <arch/types.hpp>
#include <arch/macro/mmu.hpp>
#include <arch/paging/paging_dispatcher.hpp>

namespace arch::paging {
    enum class X86PagingLevel {
        PageTable,
        PageDirectory,
        PageDirectoryPointerTable,
        PageMapLevel4,
        PageMapLevel5,
        MaxNumLevels,
    };

    struct X86PagingBase {
        typedef X86PagingLevel   LevelType;

        CXX11_CONSTEXPR 
        static auto const kTableAlignmentLog2 = 12;

        CXX11_CONSTEXPR 
        static auto const kVirtAddrExt = VirtAddrExt::Canonical;

        CXX11_CONSTEXPR
        static X86PagingLevel const kNextLevelMap[] { 
            X86PagingLevel::MaxNumLevels,
            X86PagingLevel::PageTable,
            X86PagingLevel::PageDirectory,
            X86PagingLevel::PageDirectoryPointerTable,
            X86PagingLevel::PageMapLevel4,
        };

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kNextLevel = kNextLevelMap[usize(Level)];
    };

    /// Usually active on 32-bit platform
    struct X86L2Paging: public X86PagingBase {
        template <LevelType Level>
        struct Pte;

        CXX11_CONSTEXPR
        static auto const kPagingLevel = LevelType::PageDirectory;

        CXX11_CONSTEXPR 
        static auto const kMaxPhysAddrSize = 32;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kNumPtesLog2 = 10;

        CXX11_CONSTEXPR
        static usize const kPageSizeMap[] = { KB(4), MB(4), GB(4) };

        FORCE_INLINE CXX11_CONSTEXPR
        static auto page_size(LevelType level) -> usize {
            return kPageSizeMap[usize(level)];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto max_entries(LevelType level) -> usize {
            return BIT(kNumPtesLog2<LevelType::MaxNumLevels>);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto virt_to_index(LevelType level, VirtAddr virt) -> usize {
            CXX11_CONSTEXPR 
            static usize const kMap[][2] { 
                {0, 0}, 
                {X86_PT_SHIFT, BIT(10) - 1},
                {X86_PDP_SHIFT, BIT(10) - 1},
            };

            return (virt >> kMap[usize(level)][0]) & kMap[usize(level)][1];
        }
    };

    template <>
    struct PagingDispatcher<2> {
        typedef X86L2Paging   Type;
    };

    // Page extension.
    struct X86L3Paging: public X86PagingBase {
        template <LevelType Level>
        struct Pte;

        CXX11_CONSTEXPR 
        static auto const kPagingLevel = LevelType::PageDirectoryPointerTable;

        CXX11_CONSTEXPR 
        static auto const kMaxPhysAddrSize = 36;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kNumPtesLog2 = 9;

        CXX11_CONSTEXPR
        static usize const kPageSizeMap[] = { KB(4), MB(2), GB(1), GB(4) };

        FORCE_INLINE CXX11_CONSTEXPR
        static auto max_entries(LevelType level) -> usize {
            return BIT(kNumPtesLog2<LevelType::MaxNumLevels>);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto page_size(LevelType level) -> usize {
            return kPageSizeMap[usize(level)];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(LevelType level, VirtAddr virt) -> usize {
            CXX11_CONSTEXPR 
            static usize const kMap[][2] { 
                {0, 0}, 
                {X86_PT_SHIFT, BIT(9) - 1},
                {X86_PDP_SHIFT, BIT(9) - 1},
                {X86_PDP_SHIFT, BIT(2) - 1},
            };

            return (virt >> kMap[usize(level)][0]) & kMap[usize(level)][1];
        }
    };

    template <>
    struct PagingDispatcher<3> {
        typedef X86L3Paging  Type;
    };

    /// Common to L4 and L5 paging.
    template <X86PagingLevel TopLevelV, usize MaxPhysAddrSize>
    struct X86Paging: public X86PagingBase {
        typedef X86PagingBase   Base;
        typedef typename Base::LevelType   LevelType;

        template <LevelType Level>
        struct Pte;

        CXX11_CONSTEXPR 
        static auto const kPagingLevel = TopLevelV;

        CXX11_CONSTEXPR 
        static auto const kMaxPhysAddrSize = MaxPhysAddrSize;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kNumPtesLog2 = 9;

        CXX11_CONSTEXPR
        static usize const kPageSizeMap[]
        { KB(4), MB(2), GB(1), GB(512), TB(256), PB(128) };

        FORCE_INLINE CXX11_CONSTEXPR
        static auto virt_to_index(LevelType level, VirtAddr virt) -> usize {
            CXX11_CONSTEXPR 
            static usize const kShift[] {
                0,
                X86_PT_SHIFT,
                X86_PD_SHIFT,
                X86_PDP_SHIFT,
                X86_PML4_SHIFT,
                X86_PML5_SHIFT,
            };

            return (virt >> kShift[usize(level)]) & 0x1FF;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto page_size(LevelType level) -> usize {
            return kPageSizeMap[usize(level)];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto max_entries(LevelType level) -> usize {
            return BIT(kNumPtesLog2<LevelType::MaxNumLevels>);
        }

        CXX11_CONSTEXPR
        static bool const kLevelCanBeTerminalMap[]
        { true, true, true, false, false };

        FORCE_INLINE CXX11_CONSTEXPR
        static auto level_can_be_terminal(LevelType level) -> bool {
            return kLevelCanBeTerminalMap[usize(level)];
        }
    };

    template <X86PagingLevel TopLevelV, usize MaxPhysAddrSize>
    template <X86PagingLevel Level>
    struct X86Paging<TopLevelV, MaxPhysAddrSize>::Pte {

    };

    template <>
    struct PagingDispatcher<4> {
        typedef X86Paging<X86PagingLevel::PageMapLevel4, 52>  Type;
    };

    template <>
    struct PagingDispatcher<5> {
        typedef X86Paging<X86PagingLevel::PageMapLevel5, 57>  Type;
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_X86_PAGINGS_HPP