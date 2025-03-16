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
#include <ustl/util/pair.hpp>

namespace arch::paging {
    template <typename ArchPaging>
    struct Paging {
        typedef PagingTraits<ArchPaging>    PagingTraits;
        typedef typename PagingTraits::LevelType    LevelType;

        template <typename PhysToVirt, typename Visitor>
        static auto visit_page_tables(PhysAddr table, PhysToVirt &&p2v, Visitor &&visitor, VirtAddr addr) {
        }

        template <LevelType Level, typename PhysToVirt, typename Visitor>
        static auto visit_page_tables_from(PhysAddr table, PhysToVirt &&p2v, Visitor &&visitor, VirtAddr addr) {
            VirtAddr vtable = p2v(table);
            auto const [low, high] = get_virt_addr_bit_range<Level>();
            return visit_page_tables_from<PagingTraits::NextLevel<Level>>(table, p2v, visitor, addr);
        }

        template <LevelType Level>
        static auto get_virt_addr_bit_range() -> ustl::Pair<u32, u32> {
        }
    };
} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_MOD_HPP