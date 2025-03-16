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
#ifndef OURS_PHYS_ASPACE_HPP
#define OURS_PHYS_ASPACE_HPP 1

#include <arch/paging.hpp>
#include <ours/mem/types.hpp>
#include <ours/phys/init.hpp>
#include <ktl/result.hpp>

namespace ours::phys {
    struct Aspace {
        typedef arch::Paging<void>  Paging;

        auto map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ktl::Result<usize>;

        FORCE_INLINE
        auto map_identically(VirtAddr va, usize n, mem::MmuFlags flags) -> ktl::Result<usize> {
            return map(va, n, va, flags);
        }

        auto unmap(VirtAddr va, usize n) -> Status;

        auto install() const -> void;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_ASPACE_HPP