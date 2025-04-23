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
#include <ours/mem/vm_page.hpp>
#include <ours/mem/scope.hpp>

#include <gktl/xarray.hpp>
#include <ustl/bitfields.hpp>

namespace ours::mem {
    class VmPageMap {
        enum class Tag {
            Owned,
            Borrowed = BIT(0),
        };
    public:
        FORCE_INLINE
        auto get_page(PgOff index) -> VmPage * {
            auto option = pages_.load(index).cast_to<VmPage *>();
            if (!option) {
                return nullptr;
            }
            return *option;
        }

        FORCE_INLINE
        auto insert_page(PgOff index, VmPage *page, /*states*/...) -> void {
            pages_.store(index, pages_.make_entry(page));
        }
    
    private:
        struct TempAllocator {
            template <typename T>
            FORCE_INLINE
            static auto allocate() -> T * {
                return kmalloc(sizeof(T), kGafKernel);
            }

            template <typename T>
            FORCE_INLINE
            static auto deallocate(T *t) -> void {
                kfree(t, sizeof(T), kGafKernel);
            }
        };

        gktl::Xarray<TempAllocator>  pages_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_MAP_HPP