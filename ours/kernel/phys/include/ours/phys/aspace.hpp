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

#include <ours/phys/arch-paging.hpp>
#include <arch/paging.hpp>
#include <ours/mem/types.hpp>
#include <ktl/result.hpp>
#include <ustl/option.hpp>
#include <ustl/limits.hpp>

namespace ours::phys {
    struct Aspace {
        typedef arch::Paging<ArchLowerPaging>  LowerPaging;
        typedef arch::Paging<ArchUpperPaging>  UpperPaging;
        
        CXX11_CONSTEXPR
        static auto const kDualAspace = !ustl::traits::IsSameV<LowerPaging, UpperPaging>;

        auto init() -> void {
            typedef LowerPaging::PagingTraits   LowerPagingTraits;
            lower_pgd_ = alloc_page_table(LowerPagingTraits::kSizeOfTable<LowerPagingTraits::kPagingLevel>,
                                          LowerPagingTraits::kTableAlignment);
            DEBUG_ASSERT(lower_pgd_, "");

            if CXX11_CONSTEXPR (!kDualAspace) {
                return;
            }

            typedef UpperPaging::PagingTraits   UpperPagingTraits;
            upper_pgd_ = alloc_page_table(UpperPagingTraits::kSizeOfTable<UpperPagingTraits::kPagingLevel>,
                                          UpperPagingTraits::kTableAlignment);
            DEBUG_ASSERT(lower_pgd_, "");
        }

        auto map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ktl::Result<usize>;

        FORCE_INLINE CXX11_CONSTEXPR
        auto map_identically(VirtAddr va, usize n, mem::MmuFlags flags) -> ktl::Result<usize> {
            return map(va, n, va, flags);
        }

        auto unmap(VirtAddr va, usize n) -> Status;

        auto query(VirtAddr va) -> ktl::Result<usize>;

        FORCE_INLINE CXX11_CONSTEXPR
        auto pgd() -> PhysAddr {
            return lower_pgd_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto lower_pgd() -> PhysAddr {
            return lower_pgd_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto upper_pgd() -> PhysAddr {
            return upper_pgd_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto install() const -> void {
            arch_install();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto install(PhysAddr new_pgd) -> void {
            lower_pgd_ = new_pgd;
            arch_install();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto install(PhysAddr lower_pgd, PhysAddr upper_pgd) -> void {
            lower_pgd_ = lower_pgd;
            upper_pgd_ = upper_pgd;
            arch_install();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_allocation_bounds(PhysAddr lower_bound, PhysAddr upper_bound) -> void {
            allocation_lower_bound_ = lower_bound;
            allocation_upper_bound_ = upper_bound;
        }
    private:
        /// Arch-implementation
        auto arch_install() const -> void;
        auto alloc_page_table(usize size, usize alignment) const -> PhysAddr;

        PhysAddr lower_pgd_;
        PhysAddr upper_pgd_;
        PhysAddr allocation_lower_bound_;
        PhysAddr allocation_upper_bound_;
    };

    /// Provided by architecture-specific code.
    auto arch_setup_aspace(Aspace &aspace) -> void;

    /// Identically map the ram memory under |max_limit| to virtual address space.
    auto create_identity_map_for_ram(Aspace &aspace, PhysAddr = ustl::NumericLimits<PhysAddr>::max()) -> void;

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_ASPACE_HPP