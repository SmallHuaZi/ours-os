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
#ifndef OURS_MEM_VM_PAGE_MAP_HPP
#define OURS_MEM_VM_PAGE_MAP_HPP 1

#include <ours/const.hpp>
#include <ours/mem/gaf.hpp>
#include <ours/mem/vm_page.hpp>

#include <ktl/new.hpp>
#include <ktl/allocator.hpp>
#include <ktl/xarray.hpp>
#include <ustl/bitfields.hpp>

namespace ours::mem {
    class VmPageMap {
    public:
        enum class Tag {
            Owned,
            Borrowed = BIT(0),
        };

        FORCE_INLINE
        auto get_page(PgOff index) -> VmPage * {
            auto option = pages_.load(index).cast_to<VmPage *>();
            if (!option) {
                return nullptr;
            }
            return *option;
        }

        FORCE_INLINE
        auto insert_page(PgOff index, VmPage *page, Tag tag = Tag::Owned) -> void {
            pages_.store(index, pages_.make_entry(page));
        }
    
    private:
        ktl::Xarray<ktl::Allocator<char>>  pages_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_MAP_HPP