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
#ifndef ARCH_PAGING_MOD_HPP
#define ARCH_PAGING_MOD_HPP 1

#include <arch/paging/paging_traits.hpp>
#include <arch/paging/visitors.hpp>

#include <ustl/array.hpp>
#include <ustl/util/pair.hpp>

namespace arch::paging {
    template <typename ArchPaging>
    struct Paging {
        typedef PagingTraits<ArchPaging>            PagingTraits;
        typedef typename PagingTraits::LevelType    LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        template <typename PhysToTable, typename Allocator>
        static auto map(PhysAddr table, PhysToTable &&phys_to_table, Allocator &&allocator, VirtAddr va, usize size, PhysAddr pa, 
                        MmuFlags flags) -> ustl::Result<MapError> {
            MappingVisitor<PagingTraits, Allocator> visitor(allocator, pa, size, flags);
            while (visitor.remaining_size()) {
                auto result = visit_page_tables(table, phys_to_table, visitor, va);
                if (!result) {
                    return result;
                }
            }

            return ustl::ok(MapError::None);
        }

        // TODO(SmallHuaZi): Write the body.
        template <typename PhysToTable, typename Allocator>
        static auto unmap(PhysAddr table, PhysToTable &&phys_to_table, VirtAddr va, usize n) {
            // Now it is unnecessary.
            // return visit_page_tables(table, phys_to_table, visitor, va);
        }

        template <typename PhysToVirt, typename Visitor>
        static auto visit_page_tables(PhysAddr table, PhysToVirt &&p2v, Visitor &&visitor, VirtAddr addr) {
            return visit_page_tables_from<PagingTraits::kPgingLevel>(table, p2v, visitor, addr);
        }

        template <LevelType Level, typename PhysToTable, typename Visitor>
        static auto visit_page_tables_from(PhysAddr pa_table, PhysToTable &&phys_to_table, Visitor &&visitor, VirtAddr addr) {
            Table<Level> &table = phys_to_table(pa_table);
            auto const index = PagingTraits::virt_to_index(Level, addr);
            Pte<Level> &entry = table[index];
            if (auto result = visitor(table, entry, addr)) {
                return *result;
            }
           
            if CXX17_CONSTEXPR (usize(Level) == 0) {
                OX_PANIC("Now is in the leaf of table, no more anther level.");
            }
            return visit_page_tables_from<PagingTraits::NextLevel<Level>>(entry.address(), phys_to_table, visitor, addr);
        }

        template <LevelType Level>
        static auto get_virt_addr_bit_range() -> ustl::Pair<u16, u16> {
        }
    };
} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_MOD_HPP