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

#include <arch/macro/mmu.hpp>
#include <arch/paging/mmu_flags.hpp>
#include <arch/paging/controls.hpp>
#include <arch/paging/mapping_context.hpp>
#include <arch/paging/paging_traits.hpp>
#include <arch/cache.hpp>
#include <arch/system.hpp>

#include <gktl/canary.hpp>
#include <ustl/result.hpp>
#include <ustl/bitfields.hpp>
#include <ustl/sync/atomic.hpp>

namespace arch::paging {
    class IX86PageTable {
        typedef IX86PageTable Self;
      public:
        IX86PageTable() = default;
        virtual ~IX86PageTable() = default;

        auto init(PhysAddr pa_table, VirtAddr va_table) -> Status;

        /// Maps `n` frames starting from `|pa|` to the `|va|`, and the mapping is done with the specified `|flags|`.
        /// During the mapping process, those existing entries will be handled through `|control|`.
        /// Return the number of page mappped if success.
        auto map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control) -> ustl::Result<usize, Status> {
            return this->map_pages_with_altmap(va, pa, 1, flags, control, 0);
        }

        /// Sees IX86PageTable::map_pages.
        virtual auto map_pages_with_altmap(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, Altmap *altmap)
            -> ustl::Result<usize, Status> = 0;

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        auto map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl control)
            -> ustl::Result<usize, Status> {
            return this->map_pages_bulk_with_altmap(va, pa, len, flags, control, 0);
        }

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        virtual auto map_pages_bulk_with_altmap(VirtAddr, PhysAddr *, usize len, MmuFlags flags, MapControl control, Altmap *)
            -> ustl::Result<usize, Status> = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto unmap_pages(VirtAddr va, usize n, UnMapControl) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status = 0;

        virtual auto harvest_accessed(VirtAddr va, usize n, HarvestControl control) -> Status = 0;

        virtual auto alias_to(IX86PageTable const &other, VirtAddr base, usize nr_pages) -> Status = 0;

        auto install() const -> void {
            auto cr3 = Cr3::read();
            if (cr3.get<cr3.PageTableAddress>() != phys_) {
                cr3.set<cr3.PageTableAddress>(phys_);
                cr3.write();
            }
        }

        auto page_usage() const -> usize {
            return pages_.load();
        }

      protected:
        usize flags_;
        PhysAddr phys_;
        VirtAddr virt_;

        ustl::sync::AtomicUsize pages_;
        ustl::sync::AtomicUsize refcnt_;
    };

    FORCE_INLINE
    auto IX86PageTable::init(PhysAddr pa_table, VirtAddr va_table) -> Status {
        phys_ = pa_table;
        virt_ = va_table;

        return Status::Ok;
    }

    // struct PendingTlbInvalidation {
    //     struct Item {
    //         LevelType level : 3;
    //         usize reserved : 9;
    //         VirtAddr virt_addr : 52;
    //     };

    //     auto enqueue(VirtAddr va, LevelType level) -> void {}

    //     auto clear() -> void {}

    //     usize count_;
    //     Item items_[32];
    // };

    template <typename PageTable>
    struct X86PageTableSynchroniser {
        auto sync() -> void {}

        PageTable *page_table_;
        // PendingTlbInvalidation pending_;
    };

    template <typename PageTable>
    struct MappingContext: public GenericMappingContext {
        typedef MappingContext Self;
        typedef GenericMappingContext Base;
        typedef X86PageTableSynchroniser<PageTable> Synchroniser;
        using Base::Base;

        MappingContext(VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags, usize page_size)
            : Base(va, pa, n, flags, page_size),
              altmap_(0),
              nr_mapped_(),
              synchroniser_() {}

        FORCE_INLINE CXX11_CONSTEXPR 
        auto consume(usize page_size) -> void {
            Base::consume(page_size);
            nr_mapped_ += page_size / 4096;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto mapped() -> usize {
            return nr_mapped_;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto synchroniser() -> Synchroniser & {
            return synchroniser_;
        }

        Altmap *altmap_;
        usize nr_mapped_;
        Synchroniser synchroniser_;
    };

    template <typename PagingOptions>
    struct PagingOptionsTraits {
        typedef typename PagingOptions::Mutex           Mutex;
        typedef typename PagingOptions::PhysToVirt      PhysToVirt;
        typedef typename PagingOptions::PageAllocator   PageAllocator;

        CXX11_CONSTEXPR
        static auto const kPagingLevel = PagingOptions::kPagingLevel;
    };

    /// CRTP class `X86PageTable` is a high-level implementation of architecturelly page table.
    template <typename Derived, typename Options>
    class X86PageTableImpl: public IX86PageTable {
        typedef IX86PageTable    Base;
        typedef X86PageTableImpl Self;
      protected:
        typedef PagingOptionsTraits<Options> OptionsTraits;
        typedef typename OptionsTraits::Mutex           Mutex;
        typedef typename OptionsTraits::PhysToVirt      PhysToVirt;
        typedef typename OptionsTraits::PageAllocator   PageAllocator;

        typedef typename MakePagingTraits<OptionsTraits::kPagingLevel>::Type   PagingTraits;
        typedef typename PagingTraits::LevelType    LevelType;
        typedef typename PagingTraits::template Pte<PagingTraits::kFinalLevel>  Pte;
        typedef typename Pte::ValueType    PteVal;

        typedef MappingContext<Derived> MappingContext;
      public:
        X86PageTableImpl() = default;
        virtual ~X86PageTableImpl() override = default;

        /// Sees IX86PageTable::map_pages.
        auto map_pages_with_altmap(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, Altmap *altmap)
            -> ustl::Result<usize, Status> override;

        auto map_pages_bulk_with_altmap(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl control, Altmap *altmap)
            -> ustl::Result<usize, Status> override;

        /// Sees IX86PageTable::unmap_pages.
        auto unmap_pages(VirtAddr, usize, UnMapControl) -> Status override;

        /// Sees IX86PageTable::protect_pages.
        auto protect_pages(VirtAddr, usize, MmuFlags) -> Status override;

        /// Sees IX86PageTable::query_mapping.
        auto query_mapping(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status override;

        /// Sees IX86PageTable::harvest_accessed.
        auto harvest_accessed(VirtAddr, usize, HarvestControl) -> Status override;

        auto alias_to(IX86PageTable const &other, VirtAddr base, usize nr_pages) -> Status override;

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto top_level() -> LevelType {
            return PagingTraits::kPagingLevel;
        }

      private:
        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(LevelType level, VirtAddr virt) -> usize {
            return PagingTraits::virt_to_index(level, virt);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(LevelType level) -> usize {
            return PagingTraits::page_size(level);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entries(LevelType level) -> usize {
            return PagingTraits::max_entries(level);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto alloc_page_table() -> PhysAddr {
            return 0;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_next_table_unchecked(PteVal pteval) -> PteVal volatile * {
            if (Derived::is_present(pteval) && Derived::is_large_page_mapping(pteval)) {
                return 0;
            }

            return reinterpret_cast<PteVal volatile *>(payload_.phys_to_virt(pteval & X86_PFN_MASK));
        }

        auto prepare_map_pages(VirtAddr, PhysAddr *, usize, MmuFlags, MappingContext *) -> Status;
        auto make_mapping_context(MappingContext *, VirtAddr, PhysAddr *, usize, MmuFlags) -> Status;

        /// Create a page table entry.
        auto create_mapping(LevelType, PteVal volatile *, MappingContext *, MapControl) -> Status;
        auto create_mapping_at_l0(PteVal volatile *, MappingContext *, MapControl) -> Status;

        /// Remove a page table entry.
        auto remove_mapping(PteVal volatile *, MappingContext *, usize) -> Status;
        auto remove_mapping_at_l0(PteVal *, MappingContext *) -> Status;
        auto split_mapping(PteVal volatile *) -> Status;

        /// Update a page table entry.
        auto update_mapping(PteVal volatile *, MappingContext *, usize) -> Status;
        auto update_mapping_at_l0(PteVal *, MappingContext *) -> Status;

        /// Update a entry.
        auto update_entry(LevelType, PteVal volatile *, PhysAddr, VirtAddr, MmuFlags) -> void;

        auto unmap_entry(PteVal volatile *, LevelType level) -> Status;

        struct Payload 
            : public PhysToVirt,
              public PageAllocator
        {};

        GKTL_CANARY(X86PageTable, canary_);
        Payload payload_;
        Mutex mutex_;
    };

} // namespace arch::paging

#include <arch/paging/page_table_impl.tcc>

#endif // #ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP