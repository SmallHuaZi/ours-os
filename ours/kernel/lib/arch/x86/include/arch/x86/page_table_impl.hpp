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
#ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP
#define ARCH_X86_PAGE_TABLE_IMPL_HPP 1

#include <arch/cache.hpp>
#include <arch/mapping_context.hpp>
#include <arch/x86/paging_traits.hpp>

#include <gktl/canary.hpp>
#include <ustl/result.hpp>
#include <ustl/sync/atomic.hpp>

namespace arch::x86 {
    class IX86PageTable
    {
        typedef IX86PageTable   Self;
    public:
        IX86PageTable() = default;
        virtual ~IX86PageTable() = default;

        /// Maps `n` frames starting from `|pa|` to the `|va|`, and the mapping is done with the specified `|flags|`.
        /// During the mapping process, those existing entries will be handled through `|action|`.
        /// Return the number of page mappped if success.
        virtual auto map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl action) 
            -> ustl::Result<usize, Status> = 0;

        /// Sees IX86PageTable::map_pages.
        virtual auto map_pages_with_altmap(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, Altmap *altmap) 
            -> ustl::Result<usize, Status> = 0;

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        virtual auto map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl action) 
            -> ustl::Result<usize, Status> = 0;

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        virtual auto map_pages_bulk_with_altmap(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl action) 
            -> ustl::Result<usize, Status> = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto unmap_pages(VirtAddr va, usize n) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status = 0;

        virtual auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status = 0;

        /// Calculate how many frames are required to additional cost.
        virtual auto calc_mapping_overhead(VirtAddr va, PhysAddr pa, usize n) -> usize;

        auto page_usage() const -> usize
        {  return pages_.load();  }

    protected:
        usize flags_;
        Pte volatile *phys_;
        Pte volatile *virt_;

        ustl::sync::AtomicUsize pages_;
        ustl::sync::AtomicUsize refcnt_;
    };

    struct PendingTlbInvalidation
    {
        struct Item {
            usize level: 3;
            usize reserved: 9;
            VirtAddr virt_addr: 52;
        };

        auto enqueue(VirtAddr va, usize level) -> void
        {}

        auto clear() -> void
        {}

        usize count_;
        Item items_[32];
    };

    template <typename PageTable>
    struct X86PageTableSynchroniser
    {
        auto sync() -> void;

        PageTable *page_table_;
        PendingTlbInvalidation pending_;
    };

    template <typename PageTable>
    struct MappingContext
        : public GenericMappingContext
    {
        typedef MappingContext          Self;
        typedef GenericMappingContext   Base;
        typedef X86PageTableSynchroniser<PageTable>  Synchroniser;
        using Base::Base;

        FORCE_INLINE CXX11_CONSTEXPR
        auto consume(usize page_size) -> void
        {
            Base::consume(page_size);
            nr_mapped_ += page_size / 4096;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mapped() -> usize
        {  return nr_mapped_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto synchroniser() -> Synchroniser &
        {  return synchroniser_;  }

        Altmap *altmap_;
        usize nr_mapped_;
        Synchroniser synchroniser_;
    };

    template <typename PagingOptions>
    struct PagingOptionsTraits
    {
        typedef typename PagingOptions::Mutex   Mutex;

        CXX11_CONSTEXPR 
        static usize const PAGING_LEVEL = PagingOptions::PAGING_LEVEL;
    };

    /// Class `X86PageTable` is a high-level implementation of architecturelly page table.
    template <typename Derived, typename PagingOptions>
    class X86PageTableImpl
        : public IX86PageTable
    {
        typedef X86PageTableImpl                        Self;
        typedef PagingOptionsTraits<PagingOptions>      Options;
        typedef typename Options::Mutex                 Mutex;
        typedef PagingTraits<Options::PAGING_LEVEL>     PagingTraits;
        typedef MappingContext<Derived>                 MappingContext;
    public:
        /// Sees IX86PageTable::map_pages.
        auto map_pages(VirtAddr, PhysAddr, usize, MmuFlags, MapControl) -> ustl::Result<usize, Status> override;

        /// Sees IX86PageTable::map_pages.
        auto map_pages_bulk(VirtAddr, PhysAddr *, usize, MmuFlags, MapControl) -> ustl::Result<usize, Status> override;

        /// Sees IX86PageTable::map_pages.
        auto map_pages_with_altmap(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, Altmap *altmap) 
            -> ustl::Result<usize, Status> override;

        /// Sees IX86PageTable::unmap_pages.
        auto unmap_pages(VirtAddr, usize) -> Status override;

        /// Sees IX86PageTable::protect_pages.
        auto protect_pages(VirtAddr, usize, MmuFlags) -> Status override;

        /// Sees IX86PageTable::query_mapping.
        auto query_mapping(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status override;

        /// Sees IX86PageTable::harvest_accessed.
        auto harvest_accessed(VirtAddr, usize, HarvestControl) -> Status override;

        FORCE_INLINE CXX11_CONSTEXPR
        static auto top_level() -> usize
        {  return PagingTraits::PAGE_LEVEL;  }

    private:
        FORCE_INLINE CXX11_CONSTEXPR
        static auto virt_to_index(usize level, VirtAddr virt) -> usize
        {  return PagingTraits::virt_to_index(level, virt);  }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto page_size(usize level) -> usize
        {  return PagingTraits::page_size(level);  }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto max_entries(usize level) -> usize
        {  return PagingTraits::max_entry(level);  }

        FORCE_INLINE CXX11_CONSTEXPR
        static auto alloc_page_table() -> PhysAddr 
        {  return 0; } 

        auto prepare_map_pages(VirtAddr, PhysAddr, usize, MmuFlags, MappingContext *) -> Status;
        auto prepare_map_pages_bulk(VirtAddr, PhysAddr *, usize, MmuFlags, MappingContext *) -> Status;
        auto make_mapping_context(MappingContext *, VirtAddr, PhysAddr *, usize, MmuFlags) -> Status;

        /// Create a page table entry.
        auto create_mapping(usize level, Pte volatile *, MappingContext *, MapControl) -> Status;
        auto create_mapping_at_l0(Pte volatile *, MappingContext *, MapControl) -> Status;

        /// Remove a page table entry.
        auto remove_mapping(Pte *, MappingContext *, usize) -> Status;
        auto remove_mapping_at_l0(Pte *, MappingContext *) -> Status;
        auto split_mapping(Pte volatile *) -> Status;

        /// Update a page table entry.
        auto update_mapping(Pte *, MappingContext *, usize) -> Status;
        auto update_mapping_at_l0(Pte *, MappingContext *) -> Status;

        /// Update a entry.
        auto update_entry(Pte volatile*, PhysAddr, VirtAddr, MmuFlags) -> void;

        auto unmap_entry(Pte volatile*, usize level) -> Status;

        auto populate_entry(Pte *, MappingContext *) -> Status;

        GKTL_CANARY(X86PageTable, canary_);
        Mutex  mutex_;
    };

} // namespace arch::x86

#include <arch/x86/page_table_impl.tcc>

#endif // #ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP