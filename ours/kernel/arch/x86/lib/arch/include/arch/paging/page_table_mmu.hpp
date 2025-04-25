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
#ifndef ARCH_X86_PAGE_TABLE_MMU_HPP
#define ARCH_X86_PAGE_TABLE_MMU_HPP 1

#include <arch/macro/mmu.hpp>
#include <arch/paging/page_table_impl.hpp>
#include <arch/paging/arch_mmu_flags.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace arch::paging {
    USTL_TYPE_OPTION(X86MmuPageSynchroniser, PageSynchroniser);

    template <typename Options>
    class X86PageTableMmu
        : public X86PageTableImpl<X86PageTableMmu<Options>, Options>
    {
        typedef X86PageTableMmu                  Self;
        typedef X86PageTableImpl<Self, Options>  Base;
        typedef typename Base::OptionsTraits     OptionsTraits;
        typedef typename Options::PageSynchroniser    PageSynchroniser;
        friend PageSynchroniser;
    public:
        using typename Base::PagingTraits;
        using typename Base::LevelType;

        ~X86PageTableMmu() override = default;

        using Base::init;

        auto invalidate(PendingInvalidationItems const &items) -> void {
            PageSynchroniser synchroniser{this};
            synchroniser.sync(items);
        }

        static auto check_phys_addr(PhysAddr phys_addr) -> bool {
            return false;
        }

        static auto check_virt_addr(VirtAddr virt_addr) -> bool {
            if (!is_aligned(virt_addr, PAGE_SIZE)) {
                return false;
            }
            return true;
        }

        static auto is_flags_allowed(MmuFlags) -> bool {
            return false;
        }

        FORCE_INLINE
        static auto is_present(PteVal pte) -> bool {
            return pte & X86_MMUF_PRESENT;
        }

        FORCE_INLINE
        static auto is_large_page_mapping(PteVal pte) -> bool {
            return pte & (X86_MMUF_PAGE_SIZE | X86_MMUF_PRESENT);
        }

        FORCE_INLINE
        static auto level_can_be_terminal(LevelType level) -> bool {
            return PagingTraits::level_can_be_terminal(level);
        }

        static auto make_pteval(LevelType level, PhysAddr phys, X86MmuFlags flags) -> PteVal {
            auto arch_flags = flags | X86MmuFlags::Present;
            if (level != PagingTraits::kFinalLevel) {
                arch_flags |= X86MmuFlags::PageSize;
            }

            // FIXME(SmallHuaZi) `phys` should does bit-and with address mask.
            return PteVal(PteVal(arch_flags) | phys);
        }
    };

} // namespace arch::paging

#endif // #ifndef ARCH_X86_PAGE_TABLE_MMU_HPP