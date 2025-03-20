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

#include <ours/phys/arch_paging.hpp>
#include <arch/paging.hpp>
#include <ours/mem/types.hpp>
#include <ktl/result.hpp>
#include <ustl/option.hpp>

namespace ours::phys {
    struct Aspace {
        typedef arch::Paging<ArchLowerPaging>  LowerPaging;
        typedef arch::Paging<ArchUpperPaging>  UpperPaging;

        CXX11_CONSTEXPR
        static auto const kDualAspace = !ustl::traits::IsSameV<LowerPaging, UpperPaging>;

        auto map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ktl::Result<usize>;

        FORCE_INLINE CXX11_CONSTEXPR
        auto map_identically(VirtAddr va, usize n, mem::MmuFlags flags) -> ktl::Result<usize> {
            return map(va, n, va, flags);
        }

        auto unmap(VirtAddr va, usize n) -> Status;

        /// Arch-implementation
        auto install() const -> void {
            arch_install();
        }
    private:
        auto arch_install() const -> void;
        auto alloc_page_table(usize size, usize alignment) const -> ustl::Option<PhysAddr>;

        PhysAddr pgd_;
        PhysAddr allocation_lower_bound_;
        PhysAddr allocation_upper_bound_;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_ASPACE_HPP