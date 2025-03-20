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
#ifndef ARCH_PAGING_VISITORS_HPP
#define ARCH_PAGING_VISITORS_HPP 1

#include <arch/paging/mapping_context.hpp>
#include <arch/paging/arch_mmu_flags.hpp>

#include <ustl/array.hpp>
#include <ustl/option.hpp>

namespace arch::paging {
    enum class MapError {
        None,
        Existent,
        NoMem,
    };

    /// Handler for doing a times of the mapping action.
    template <typename PagingTraits, typename Allocator>
    struct MappingVisitor {
        typedef typename PagingTraits::LevelType                LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        MappingVisitor(Allocator &&allocator, PhysAddr phys_addr, usize size, MmuFlags flags)
            : allocator_(allocator),
              phys_addr_(phys_addr),
              phys_cursor_(&phys_addr_, 1, size)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto remaining_size() const -> usize {
            return phys_cursor_.remaining_size();
        }

        template <LevelType Level>
        auto operator()(Table<Level> &table, Pte<Level> &pte, VirtAddr addr) -> ustl::Option<MapError> {
            if (pte.is_present()) {
                if (pte.is_terminal()) {
                    return MapError::Existent;
                }
            }
            CXX11_CONSTEXPR
            auto const page_size = PagingTraits::page_size(Level);
            auto const start_address = phys_cursor_.phys_addr();
            auto const remaining_size = phys_cursor_.remaining_size();

            // Can we take a large page mapping?
            CXX11_CONSTEXPR
            auto const is_terminal = PagingTraits::level_can_be_terminal(Level) &&
                                     page_size < remaining_size &&
                                     start_address % page_size == 0 &&
                                     addr % page_size = 0;

            PhysAddr phys_addr;
            // Non-terminal entry
            if (is_terminal) {
                phys_addr = start_address;
                phys_cursor_.consume(page_size);
            } else {
                phys_addr = allocator_(page_size, PagingTraits::kTableAlignment);
                if (!phys_addr) {
                    return MapError::NoMem;
                }
            }

            auto const flags = down_cast<ArchMmuFlags>(flags_);
            pte = Pte<Level>::make(phys_addr, flags);

            if (is_terminal) {
                return MapError::None;
            }
            return ustl::NONE;
        }

        MmuFlags flags_;
        Allocator allocator_;
        PhysAddr phys_addr_;
        PhysAddrCursor phys_cursor_;
    };

    template <typename PagingTraits, typename Allocator>
    struct UnMapVisitor {
        typedef typename PagingTraits::LevelType                LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        UnMapVisitor(Allocator &&allocator, PhysAddr phys_addr, usize size, MmuFlags flags) {
        }
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_VISITORS_HPP