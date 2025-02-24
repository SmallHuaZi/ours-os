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

#include <arch/x86/page_table_impl.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace arch::x86 {
    template <typename Options>
    class X86PageTableMmu
        : public X86PageTableImpl<X86PageTableMmu<Options>, Options>
    {
        typedef X86PageTableMmu                  Self;
        typedef X86PageTableImpl<Self, Options>  Base;

        template <usize Level>
        using PageLevelTag = ustl::traits::IntegralConstant<usize, Level>;
    public:
        auto init() -> Status;

        auto init(PhysAddr pt, VirtAddr vt) -> Status;

        auto alias_kernel_mappings() -> Status;

        auto invalidate_tlb() -> void;

        static auto check_phys_addr(PhysAddr) -> bool;
        static auto check_virt_addr(VirtAddr) -> bool;
        static auto is_flags_allowed(MmuFlags) -> bool;

        static auto is_present(Pte pte) -> bool;
        static auto is_large_page_mapping(Pte pte) -> bool;

        static auto has_supported_page_size(usize level) -> bool
        {  return Self::priv_has_supported_page_size(level, PageLevelTag<Base::top_level()>());  }

        static auto make_pte(PhysAddr phys, MmuFlags flags) -> Pte
        {  return Self::priv_make_pte(phys, flags, PageLevelTag<Base::top_level()>());  }

        static auto get_next_table_unchecked(ai_virt Pte volatile *pte) -> Pte volatile *
        {  return Self::priv_get_next_table_unchecked(0, PageLevelTag<Base::top_level()>());  }
    private:
        static auto priv_make_pte(PhysAddr phys, MmuFlags flags, PageLevelTag<4>)
        {  return }

        static auto priv_make_pte(PhysAddr phys, MmuFlags flags, PageLevelTag<5>)
        {}

        static auto priv_get_next_table_unchecked(Pte volatile *pte, PageLevelTag<4>) -> Pte volatile *
        {}

        static auto priv_get_next_table_unchecked(Pte volatile *pte, PageLevelTag<5>) -> Pte volatile *
        {}

        static auto priv_has_supported_page_size(usize level, PageLevelTag<4>) -> bool
        {}

        static auto priv_has_supported_page_size(usize level, PageLevelTag<5>) -> bool
        {}
    };

} // namespace arch::x86

#endif // #ifndef ARCH_X86_PAGE_TABLE_MMU_HPP