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

#ifndef ARCH_X86_PAGE_TABLE_EPT_HPP
#define ARCH_X86_PAGE_TABLE_EPT_HPP 1

#include <arch/x86/page_table_impl.hpp>

namespace arch::x86 {
    template <typename Options>
    class X86PageTableEpt
        : public X86PageTableImpl<X86PageTableEpt<Options>, Options>
    {
        typedef X86PageTableEpt     Self;
        typedef X86PageTableImpl<Self, Options> Base;
    public:
        ~X86PageTableEpt() override = default;

        auto init() -> Status
        { return Status::Unimplemented;  }

        static auto check_phys_addr(PhysAddr) -> bool
        { return false;  }
        static auto check_virt_addr(VirtAddr) -> bool
        { return false;  }
        static auto is_flags_allowed(MmuFlags) -> bool
        { return false;  }

        static auto is_large_page_mapping(Pte pte) -> bool
        { return false;  }

        static auto is_present(Pte pte) -> bool
        { return false;  }
        static auto has_supported_page_size(usize level) -> bool
        { return false;  }
        static auto make_pte(PhysAddr phys, MmuFlags flags) -> Pte
        { return {};  }

        static auto get_next_table_unchecked(ai_virt Pte volatile *pte) -> Pte volatile *
        { return {};  }
    };

} // namespace arch::x86

#endif // #ifndef ARCH_X86_PAGE_TABLE_EPT_HPP