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
#include <arch/paging/arch_mmu_flags.hpp>
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
        static X86PagingLevel const kAllLevels[] { 
            X86PagingLevel::PageTable,
            X86PagingLevel::PageDirectory,
            X86PagingLevel::PageDirectoryPointerTable,
            X86PagingLevel::PageMapLevel4,
            X86PagingLevel::PageMapLevel5,
        };
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
    struct PagingDispatcher<X86PagingLevel::PageDirectory> {
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
    struct PagingDispatcher<X86PagingLevel::PageDirectoryPointerTable> {
        typedef X86L3Paging  Type;
    };

    template <X86PagingLevel TopLevelV, X86PagingLevel Level>
    struct X86Pte {
        typedef PhysAddr   ValueType;

        FORCE_INLINE CXX11_CONSTEXPR
        static auto make(ValueType addr, X86MmuFlags flags, bool terminal) -> X86Pte {
            if (!terminal) { 
                return { (addr & X86_PFN_MASK) | ValueType(flags) | X86_KERNEL_PD_FLAGS };
            }
            
            if CXX17_CONSTEXPR (Level == X86PagingLevel::PageTable) {
                return { (addr & X86_PFN_MASK) | ValueType(flags) | X86_KERNEL_PAGE_FLAGS };
            } else {
                return { (addr & X86_PFN_MASK) | ValueType(flags) | X86_KERNEL_HUGE_PAGE_FLAGS };
            }
        }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto make_terminal(ValueType addr, X86MmuFlags flags) -> X86Pte {
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto address() const -> PhysAddr {
            return inner_ & X86_PFN_MASK;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_present() const -> bool {
            return inner_ & X86_MMUF_PRESENT;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_terminal() const -> bool {
            if (!is_present()) {
                return false;
            }
            if CXX17_CONSTEXPR (Level == X86PagingLevel::PageTable) {
                return true;
            }
            if CXX11_CONSTEXPR (Level == X86PagingLevel::PageDirectory || 
                                Level == X86PagingLevel::PageDirectoryPointerTable) {
                return (inner_ & X86_MMUF_PAGE_SIZE);
            }
            return false;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto arch_mmu_flags() -> X86MmuFlags {
            static_assert(sizeof(X86_MMUF_MASK) == sizeof(inner_));
            static_assert(sizeof(X86MmuFlags) == sizeof(inner_));
            return X86MmuFlags(inner_ & X86_MMUF_MASK);
        }

        PteVal inner_;
    };

    /// Common to L4 and L5 paging.
    template <X86PagingLevel TopLevelV, usize MaxPhysAddrSize>
    struct X86Paging: public X86PagingBase {
        typedef X86PagingBase   Base;
        typedef typename Base::LevelType   LevelType;

        template <LevelType Level>
        using Pte = X86Pte<TopLevelV, Level>;

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

    template <>
    struct PagingDispatcher<X86PagingLevel::PageMapLevel4> {
        typedef X86Paging<X86PagingLevel::PageMapLevel4, 52>  Type;
    };

    template <>
    struct PagingDispatcher<X86PagingLevel::PageMapLevel5> {
        typedef X86Paging<X86PagingLevel::PageMapLevel5, 57>  Type;
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_X86_PAGINGS_HPP