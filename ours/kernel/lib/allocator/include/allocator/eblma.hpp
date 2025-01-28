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

#include <ustl/algorithms/search.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/collections/intrusive/slist.hpp>

namespace allocator {
    /// Embeded block list memory allocator.
    struct Eblma
    {
        auto do_allocate() -> void *;
        auto do_deallocate() -> void;

        auto add(size_t base, size_t size, size_t flags) -> void
        {
            auto block = reinterpret_cast<BlockUnit *>(base);
            ustl::mem::construct_at(block, base, size, flags);

            auto first = free_list_.begin(), last = free_list_.end();
            for (auto ; condition; inc-expression) {
            
            }

            auto first_lesser = ustl::algorithms::find_if(free_list_.begin(), free_list_.end(), [base, size] (BlockUnit &block) {
                return base + (size - 1) < block.base;
            });
        }

        struct BlockUnit
            : ustl::collections::intrusive::SlistBaseHook<>
        {
            BlockUnit(size_t b, size_t s, size_t f)
                : base(b), size(s), flags(f)
            {}

            size_t base;
            size_t size; 
            size_t flags;
        };

        typedef ustl::collections::intrusive::Slist<BlockUnit>  BlockList;
        size_t total_;
        BlockList free_list_;
        BlockList busy_list_;
    };

} // namespace allocator

#endif // #ifndef ALLOCATOR_EBLIST_HPP