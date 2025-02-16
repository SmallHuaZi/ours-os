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

#ifndef ALLOCATOR_EBLIST_HPP
#define ALLOCATOR_EBLIST_HPP 1

#include <bootmem/region.hpp>

#include <ustl/algorithms/search.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/collections/intrusive/slist.hpp>

namespace bootmem {
    /// Embeded block list memory allocator.
    struct Eblma
    {
        typedef Eblma       Self;

        struct BlockUnit
            : public Region
        {
            typedef Region          Base;
            typedef BlockUnit       Self;

            BlockUnit(usize b, usize s, usize f)
                : Region(b, s, f), managed_hook()
            {}

            ustl::collections::intrusive::SlistBaseHook<> managed_hook;
            USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedOptions);
        };
        USTL_DECLARE_SLIST(BlockUnit, BlockList, BlockUnit::ManagedOptions); 

        usize total_;
        BlockList free_list_;
        BlockList busy_list_;
    };

} // namespace bootmem

#endif // #ifndef ALLOCATOR_EBLIST_HPP