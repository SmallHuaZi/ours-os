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
    struct QueryResult {
        PhysAddr phys;
        usize size;
        MmuFlags flags;
    };

    template <typename PagingTraits>
    struct QueryVisitor {
        typedef typename PagingTraits::LevelType                LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        template <LevelType Level>
        auto operator()(Table<Level> &table, Pte<Level> &pte, VirtAddr addr) -> ustl::Option<QueryResult> {
            if (!pte.is_present()) {
                return ustl::none();
            }

            if (!pte.is_terminal()) {
                return ustl::none();
            }

            QueryResult result;
            result.size = PagingTraits::page_size(Level);
            result.flags = mmuflags_cast(pte.arch_mmu_flags());
            result.phys = pte.address();

            return ustl::some(result);
        }
    };

    enum class MapError {
        Existent,
        NoMem,
    };

    FORCE_INLINE CXX11_CONSTEXPR
    auto to_string(MapError error) -> char const * {
        switch (error) {
            case MapError::Existent:return "Existent";
            case MapError::NoMem: return "No memory";
        }

        return "Unknown map error";
    }

    /// Handler for doing a times of the mapping action.
    template <typename PagingTraits, typename Allocator>
    struct MappingVisitor {
        typedef typename PagingTraits::LevelType                LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        MappingVisitor(Allocator &&allocator, VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags, usize page_size)
            : allocator_(allocator),
              context_(va, pa, n, flags, page_size)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto has_more() const -> usize {
            return context_.has_more();
        }

        template <LevelType Level>
        auto operator()(Table<Level> &table, Pte<Level> &pte, VirtAddr addr) 
            -> ustl::Option<ustl::Result<void, MapError>> {
            if (pte.is_present()) {
                if (pte.is_terminal()) {
                    return ustl::err(MapError::Existent);
                }

                return ustl::none();
            }
            // CXX11_CONSTEXPR
            auto const page_size = PagingTraits::page_size(Level);
            auto const start_address = context_.phys_addr();
            auto const remaining_size = context_.remaining_size();

            // Can we take a large page mapping?
            // 1. Hardware support.
            // 2. Size enough.
            // 3. Physical and virutal address alignment.
            auto const is_terminal = PagingTraits::level_can_be_terminal(Level) &&
                                     page_size <= remaining_size &&
                                     start_address % page_size == 0 &&
                                     addr % page_size == 0;

            PhysAddr phys_addr;
            // Non-terminal entry
            if (is_terminal) {
                phys_addr = start_address;
                context_.consume(page_size);
            } else {
                CXX11_CONSTEXPR auto const table_size = PagingTraits::template kSizeOfTable<Level>;
                phys_addr = allocator_(table_size, PagingTraits::kTableAlignment);
                if (!phys_addr) {
                    return ustl::err(MapError::NoMem);
                }
            }

            pte = Pte<Level>::make(phys_addr, context_.flags(), is_terminal);

            if (is_terminal) {
                return ustl::ok();
            }

            return ustl::none();
        }

        Allocator allocator_;
        MapContext context_;
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