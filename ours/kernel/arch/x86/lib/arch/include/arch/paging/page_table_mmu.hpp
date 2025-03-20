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

        template <usize Level>
        using PageLevelTag = ustl::traits::IntegralConstant<usize, Level>;
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

        static auto is_present(Pte pte) -> bool
        {  return pte & X86_MMUF_PRESENT;  }

        static auto is_large_page_mapping(Pte pte) -> bool
        {  return pte & X86_MMUF_PAGE_SIZE;  }

        static auto level_can_be_terminal(LevelType level) -> bool
        {  return PagingTraits::level_can_be_terminal(level); }

        static auto make_pte(PhysAddr phys, MmuFlags flags) -> Pte
        {  return Self::priv_make_pte(phys, flags, PageLevelTag<Base::top_level()>());  }

        static auto get_next_table_unchecked(ai_virt Pte volatile *pte) -> Pte volatile *
        {  return Self::priv_get_next_table_unchecked(0, PageLevelTag<Base::top_level()>());  }
    private:
        static auto priv_make_pte(PhysAddr phys, MmuFlags flags, PageLevelTag<4>)
        {  return 0; }

        static auto priv_make_pte(PhysAddr phys, MmuFlags flags, PageLevelTag<5>)
        {  return 0; }

        static auto priv_get_next_table_unchecked(Pte volatile *pte, PageLevelTag<4>) -> Pte volatile *
        {  return 0; }

        static auto priv_get_next_table_unchecked(Pte volatile *pte, PageLevelTag<5>) -> Pte volatile *
        {  return 0; }
    };

} // namespace arch::paging 

#endif // #ifndef ARCH_X86_PAGE_TABLE_MMU_HPP