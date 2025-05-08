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
#include <arch/paging/arch_mmu_flags.hpp>
#include <arch/paging/controls.hpp>
#include <arch/paging/mapping_context.hpp>
#include <arch/paging/paging_traits.hpp>
#include <arch/paging/x86_pagings.hpp>
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

        /// Maps `n` frames starting from `|pa|` to the `|va|`, and the mapping is done with the specified `|flags|`.
        /// During the mapping process, those existing entries will be handled through `|control|`.
        /// Return the number of page mappped if success.
        virtual auto map_pages(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, usize *mapped) -> Status = 0;

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        virtual auto map_pages_bulk(VirtAddr, PhysAddr *, usize len, MmuFlags flags, MapControl control, usize *mapped)
            -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto unmap_pages(VirtAddr va, usize n, UnmapControl, usize *unmapped) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status = 0;

        virtual auto harvest_accessed(VirtAddr va, usize n, HarvestControl control) -> Status = 0;

        virtual auto alias_to(IX86PageTable const &other, VirtAddr base, usize nr_pages) -> Status = 0;

        FORCE_INLINE
        auto install() const -> void {
            auto cr3 = Cr3::read();
            if (cr3.get<cr3.PageTableAddress>() != phys_) {
                cr3.set<cr3.PageTableAddress>(phys_);
                cr3.write();
            }
        }

        FORCE_INLINE
        auto page_usage() const -> usize {
            return pages_.load();
        }

        FORCE_INLINE
        auto pgd_phys() const -> PhysAddr {
            return phys_;
        }
      protected:
        usize flags_;
        PhysAddr phys_;
        VirtAddr virt_;

        ustl::sync::AtomicUsize pages_;
        ustl::sync::AtomicUsize refcnt_;
    };

    struct PendingInvalidationItems {
        typedef X86PagingLevel    LevelType;
        enum FieldId: usize {
            Level,
            Global,
            Terminal,
            Addr
        };

        template <FieldId Id, usize Bits, typename Type, bool Natural = false>
        using Field = ustl::bitfields::Field<
                            ustl::bitfields::StorageUnit<usize>,
                            ustl::bitfields::Id<Id>,
                            ustl::bitfields::Bits<Bits>,
                            ustl::bitfields::Type<Type>,
                            ustl::bitfields::Natural<Natural>>;
        
        typedef ustl::BitFields<
            Field<Level, ustl::bit_width(usize(LevelType::MaxNumLevels)), LevelType>,
            Field<Global, 1, bool>,
            Field<Terminal, 1, bool>,
            Field<Addr, ustl::NumericLimits<VirtAddr>::DIGITS - PAGE_SHIFT, VirtAddr, true>
         > Item;

        CXX11_CONSTEXPR
        static auto const kMaxPendingItems = 32;

        FORCE_INLINE CXX11_CONSTEXPR
        auto append(VirtAddr addr, LevelType level, bool is_global, bool is_terminal) -> void {
            items_[count_].set<Level>(level);
            items_[count_].set<Global>(is_global);
            items_[count_].set<Terminal>(is_terminal);
            items_[count_].set<Addr>(addr);
            count_ += 1;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto clear() -> void {
            count_ = 0;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_full() const -> bool {
            return count_ == kMaxPendingItems;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto count() const -> usize {
            return count_;
        }

        usize count_;
        Item items_[kMaxPendingItems];
    };

    template <typename PageTableOptions>
    struct PageTableOptionsTraits {
        typedef typename PageTableOptions::Mutex            Mutex;
        typedef typename PageTableOptions::PhysToVirt       PhysToVirt;
        typedef typename PageTableOptions::PageAllocator    PageAllocator;

        CXX11_CONSTEXPR
        static auto const kPagingLevel = PageTableOptions::kPagingLevel;
    };

    /// CRTP class `X86PageTable` is a high-level implementation of architecturelly page table.
    template <typename Derived, typename Options>
    class X86PageTableImpl: public IX86PageTable {
        typedef IX86PageTable    Base;
        typedef X86PageTableImpl Self;
      protected:
        class PageSynchroniser;

        typedef PageTableOptionsTraits<Options>             OptionsTraits;
        typedef typename OptionsTraits::Mutex               Mutex;
        typedef typename OptionsTraits::PhysToVirt          PhysToVirt;
        typedef typename OptionsTraits::PageAllocator       PageAllocator;

        typedef typename MakePagingTraits<OptionsTraits::kPagingLevel>::Type   PagingTraits;
        typedef typename PagingTraits::LevelType    LevelType;
        typedef typename PagingTraits::template Pte<PagingTraits::kFinalLevel>  Pte;
        typedef typename Pte::ValueType    PteVal;
      public:
        X86PageTableImpl() = default;
        virtual ~X86PageTableImpl() override = default;

        auto init() -> Status;

        auto init(PhysAddr pa_table, VirtAddr va_table) -> Status;

        /// Sees IX86PageTable::map_pages.
        auto map_pages(VirtAddr, PhysAddr, usize, MmuFlags, MapControl, usize *) -> Status override;

        auto map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl control, usize *)
            -> Status override;

        /// Sees IX86PageTable::unmap_pages.
        auto unmap_pages(VirtAddr, usize, UnmapControl, usize *unmapped) -> Status override;

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
        auto alloc_page_table() -> PhysAddr {
            // FIXME(SmallHuaZi)
            return get_allocator().alloc_pages(1, PAGE_SIZE);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto phys_to_virt(PhysAddr phys_addr) -> VirtAddr {
            return payload_.phys_to_virt(phys_addr);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_next_table_unchecked(PteVal pteval) -> PteVal volatile * {
            if (!Derived::is_present(pteval) || Derived::is_large_page_mapping(pteval)) {
                return 0;
            }

            return reinterpret_cast<PteVal volatile *>(phys_to_virt(pteval & X86_PG_FRAME));
        }

        auto prepare_map_pages(VirtAddr, PhysAddr *, usize, MmuFlags, MapContext *) -> Status;
        auto make_mapping_context(MapContext *, VirtAddr, PhysAddr *, usize, MmuFlags) -> Status;

        /// Create a page table entry.
        auto create_mapping(LevelType, PteVal volatile *, MapContext *, MapControl, PageSynchroniser *) -> Status;
        auto create_mapping_at_l0(PteVal volatile *, MapContext *, MapControl, PageSynchroniser *) -> Status;

        /// Remove a page table entry.
        auto remove_mapping(LevelType, PteVal volatile *, TravelContext *, UnmapControl, PageSynchroniser *) -> Status;
        auto remove_mapping_at_l0(PteVal volatile*, TravelContext *, UnmapControl, PageSynchroniser *) -> Status;

        /// Split a large page mapping into multiple smaller page mappings.
        auto split_mapping(LevelType, PteVal volatile *, VirtAddr va, TravelContext *, PageSynchroniser *) -> Status;

        auto read_mapping(PteVal volatile *, VirtAddr, LevelType *out_level, PteVal *out_pte) -> Status;

        /// Update a page table entry.
        auto update_mapping(LevelType, PteVal volatile *, TravelContext *, PageSynchroniser *) -> Status;
        auto update_mapping_at_l0(PteVal volatile *, TravelContext *, PageSynchroniser *) -> Status;

        /// Update a entry.
        auto update_entry(PteVal volatile *, LevelType, PhysAddr, VirtAddr, X86MmuFlags, PageSynchroniser *, bool terminal) -> void;

        auto unmap_entry(PteVal volatile *, LevelType level, VirtAddr, PageSynchroniser *) -> void;

        FORCE_INLINE
        auto get_allocator() -> PageAllocator & {
            return static_cast<PageAllocator &>(payload_);
        }

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