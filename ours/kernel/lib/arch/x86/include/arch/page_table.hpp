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
#ifndef ARCH_X86_PAGE_TABLE_HPP
#define ARCH_X86_PAGE_TABLE_HPP 1

#include <arch/x86/page_table_mmu.hpp>
#include <arch/x86/page_table_ept.hpp>
#include <arch/x86/paging_traits.hpp>
#include <arch/options.hpp>

#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/views/span.hpp>
#include <ustl/placeholders.hpp>
#include <ustl/util/pack_options.hpp>

#include <gktl/canary.hpp>

namespace arch {
namespace x86 {
    /// `PageTable` is a high-level abstract to virutal memory mapping.
    /// `|Mutex|` should provides the interface `lock` and `unlock` at least.
    /// `|PageManager|`  should provides the interface `alloc_page` and `free_page`  at least.
    template <typename Options>
    class X86PageTable
    {
    public:
        auto init(VmasFlags flags) -> Status;

        ai_unsafe auto init(PhysAddr pa_table, VirtAddr va_table, VmasFlags flags) -> Status 
        {  return pimpl_->init((Pte volatile *)pa_table, (Pte volatile *)va_table, flags);  }

        /// Sees IX86PageTable::map_pages.
        FORCE_INLINE
        auto map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl action) -> ustl::Result<usize, Status>
        {  return pimpl_->map_pages(va, pa, n, flags, action);  }

        /// Sees IX86PageTable::unmap_pages.
        FORCE_INLINE
        auto unmap_pages(VirtAddr va, usize n, UnMapControl control) -> Status
        {  return pimpl_->unmap_pages(va, n, control);  }

        /// Sees IX86PageTable::protect_pages.
        FORCE_INLINE
        auto protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status
        {  return pimpl_->protect_pages(va, n, flags);  }

        /// Sees IX86PageTable::query_mapping.
        FORCE_INLINE
        auto query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status
        {  return pimpl_->query_mapping(va, pa, flags);  }

        /// Sees IX86PageTable::harvest_accessed.
        FORCE_INLINE
        auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status
        {  return pimpl_->harvest_accessed(va, n, action);  }

        FORCE_INLINE
        auto alias_to(X86PageTable const &other, VirtAddr base, usize nr_pages, usize level) -> Status
        {  return pimpl_->alias_to(other, base, nr_pages, level);  }

    private:
        using Mmu = x86::X86PageTableMmu<Options>;
        using Ept = x86::X86PageTableEpt<Options>;

        union Storage {
            alignas(Mmu) char mmu_[sizeof(Mmu)];
            alignas(Ept) char ept_[sizeof(Ept)];
        } storage_;
        x86::IX86PageTable *pimpl_;
    };

    template <typename Options>
    auto X86PageTable<Options>::init(VmasFlags flags) -> Status
    {
        if (bool(flags & VmasFlags::Guest)) {
            Ept *ept= new (&storage_.ept_) Ept();
            auto status = ept->init();
            if (status != Status::Ok) {
                return status;
            }
        } else {
            Mmu *mmu = new (&storage_.mmu_) Mmu();
            auto status = mmu->init();
            if (status != Status::Ok) {
                return status;
            }
        }

        return Status::Ok;
    }

    struct PageTableDefaultOptions
    {
        CXX11_CONSTEXPR
        static usize const PAGING_LEVEL = 4;

        typedef ustl::sync::Mutex   Mutex;
        typedef void  PageManager;
        typedef void  TlbInvalidator;
    };

    template <typename... Options>
    struct MakeX86PageTable
    {
        typedef typename ustl::PackOptions<PageTableDefaultOptions, Options...>::Type
            PackedOptions;

        typedef X86PageTable<PackedOptions>     Type;
    };
} // namespace x86 

    /// The `PageTable` type alias follows standard naming conventions for generic page table implementations.
    /// It supports the following template parameters:
    ///     1) \c `Mutex<>`       - Locking mechanism for concurrent access
    ///     2) \c `PageManager<>` - Physical page allocation strategy
    ///     3) \c `TlbInvalidator<>` - TLB shootdown handling
    ///     4) \c `PagingConfig<>` - Paging config 
    ///
    /// These parameters allow customization of:
    ///   - Synchronization primitives
    ///   - Memory management policies
    ///   - Cache coherence protocols
    template <typename... Options>
    using PageTable = typename x86::MakeX86PageTable<Options...>::Type;

} // namespace arch

#endif // #ifndef ARCH_X86_PAGE_TABLE_HPP