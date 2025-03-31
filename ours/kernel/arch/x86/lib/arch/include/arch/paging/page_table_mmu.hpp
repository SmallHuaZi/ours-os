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
    template <typename Options>
    class X86PageTableMmu
        : public X86PageTableImpl<X86PageTableMmu<Options>, Options>
    {
        typedef X86PageTableMmu                  Self;
        typedef X86PageTableImpl<Self, Options>  Base;

        using typename Base::PagingTraits;
        using typename Base::LevelType;
    public:
        ~X86PageTableMmu() override = default;

        auto init() -> Status
        {  return Status::Unimplemented;  }

        auto invalidate_tlb() -> void;

        static auto check_phys_addr(PhysAddr) -> bool
        {  return false; }

        static auto check_virt_addr(VirtAddr) -> bool
        {  return false; }

        static auto is_flags_allowed(MmuFlags) -> bool
        {  return false; }

        static auto is_present(PteVal pte) -> bool
        {  return pte & X86_MMUF_PRESENT;  }

        static auto is_large_page_mapping(PteVal pte) -> bool
        {  return pte & X86_MMUF_PAGE_SIZE;  }

        static auto level_can_be_terminal(LevelType level) -> bool
        {  return PagingTraits::level_can_be_terminal(level); }

        static auto make_pteval(LevelType level, PhysAddr phys, MmuFlags flags) -> PteVal {
            auto arch_flags = mmuflags_cast<X86MmuFlags>(flags) | X86MmuFlags::Present;
            if (level != PagingTraits::kFinalLevel) {
                arch_flags |= X86MmuFlags::PageSize;
            }

            // FIXME(SmallHuaZi) `phys` should does bit-and with address mask.
            return PteVal(PteVal(arch_flags) | phys);
        }
    };

} // namespace arch::paging 

#endif // #ifndef ARCH_X86_PAGE_TABLE_MMU_HPP