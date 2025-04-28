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

#include <arch/paging/page_table_impl.hpp>

namespace arch::paging {
    USTL_TYPE_OPTION(X86EptPageSynchroniser, EptPageSynchroniser);

    template <typename Options>
    class X86PageTableEpt
        : public X86PageTableImpl<X86PageTableEpt<Options>, Options>
    {
        typedef X86PageTableEpt     Self;
        typedef X86PageTableImpl<Self, Options> Base;
    public:
        using typename Base::PagingTraits;
        using typename Base::LevelType;

        virtual ~X86PageTableEpt() override = default;

        using Base::init;

        auto invalidate(PendingInvalidationItems const &items) -> void {
        }

        static auto check_phys_addr(PhysAddr) -> bool
        { return false;  }
        static auto check_virt_addr(VirtAddr) -> bool
        { return false;  }
        static auto is_flags_allowed(MmuFlags) -> bool
        { return false;  }

        static auto is_large_page_mapping(PteVal pte) -> bool
        { return false;  }

        static auto is_present(PteVal pte) -> bool
        { return false;  }

        static auto level_can_be_terminal(LevelType level) -> bool
        { return false;  }

        FORCE_INLINE
        static auto interminal_mmuflags() -> ArchMmuFlags {
            return ArchMmuFlags{};
        }

        FORCE_INLINE
        static auto make_terminal_mmuflags(LevelType level, ArchMmuFlags mmuflags) -> ArchMmuFlags {
            return mmuflags;
        }

        static auto make_pteval(PhysAddr phys, X86MmuFlags flags) -> PteVal
        { return {};  }
    };

} // namespace arch::paging

#endif // #ifndef ARCH_X86_PAGE_TABLE_EPT_HPP