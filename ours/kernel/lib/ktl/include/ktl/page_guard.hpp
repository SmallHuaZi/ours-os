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
#ifndef INCLUDE_KTL_PAGE_GUARD_HPP
#define INCLUDE_KTL_PAGE_GUARD_HPP

#include <ours/assert.hpp>
#include <ours/types.hpp>

#include <ours/mem/cfg.hpp>
#include <ours/mem/pmm.hpp>

#include <ustl/mem/align.hpp>

namespace gktl {
    enum class AllocationStrategy {
        EarlyMem,
        Page,
    };

    template <typename T>
    struct PageGuard {
        FORCE_INLINE CXX11_CONSTEXPR
        PageGuard(T *page) {
            DEBUG_ASSERT(ustl::mem::is_aligned(page, PAGE_SIZE));
            page_ = page;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        ~PageGuard() {
            ours::mem::free_frame(ours::VirtAddr(page_));
        }

        auto data() -> T * {
            DEBUG_ASSERT(page_);
            return page_;
        }
        
        auto data() const -> T const * {
            DEBUG_ASSERT(page_);
            return page_;
        }

        operator T*() {
            return data();
        }

        operator T*() const {
            return data();
        }
        
        T *page_ = nullptr;
        usize order = 0;
    };

    template <typename T>
    PageGuard(T *) -> PageGuard<T>;

} // namespace gktl

#endif // INCLUDE_KTL_PAGE_GUARD_HPP
