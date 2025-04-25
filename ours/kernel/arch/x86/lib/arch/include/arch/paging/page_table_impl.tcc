#ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP
#   include <arch/paging/page_table_impl.hpp>
#endif // #ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP

#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/sync/lockguard.hpp>

#define TEMPLATE \
    template <typename Derived, typename PagingOptions>

#define X86_PAGE_TABLE \
    X86PageTableImpl<Derived, PagingOptions>

namespace arch::paging {
    using ustl::mem::is_aligned;
    using ustl::mem::align_down;
    using ustl::mem::align_up;

    TEMPLATE
    class X86_PAGE_TABLE::PageSynchroniser {
      public:
        auto append(VirtAddr addr, LevelType level, bool is_global, bool is_terminal) -> void {
            items_.append(addr, level, is_global, is_terminal);
            if (items_.is_full()) {
                sync();
            }
        }

        auto sync() -> void {
            page_table_->invalidate(items_);
        }
    
      private:
        Derived *page_table_;
        PendingInvalidationItems items_;
    };

    TEMPLATE
    auto X86_PAGE_TABLE::init() -> Status {
        PhysAddr pgd_pa = alloc_page_table();
        if (!phys_) {
            return Status::OutOfMem;
        }

        return init(pgd_pa, phys_to_virt(pgd_pa));
    }

    TEMPLATE
    auto X86_PAGE_TABLE::init(PhysAddr pgd_pa, VirtAddr pgd_va) -> Status {
        phys_ = pgd_pa; 
        virt_ = pgd_va;
        return Status::Ok;
    }

    TEMPLATE FORCE_INLINE
    auto X86_PAGE_TABLE::unmap_entry(PteVal volatile *pteval, LevelType level, VirtAddr virt, 
                                     PageSynchroniser *synchroniser) -> Status {  
        auto const derived = static_cast<Derived *>(this);
        PteVal const old_pte = *pteval;
        *pteval = 0;

        if (Derived::is_present(old_pte)) {
            synchroniser->append(virt, level, *pteval & X86_MMUF_GLOBAL, Derived::is_large_page_mapping(old_pte));
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::update_entry(PteVal volatile *pteval, LevelType level, PhysAddr phys, VirtAddr virt, 
                                      X86MmuFlags flags, PageSynchroniser *synchroniser) -> void {
        auto const derived = static_cast<Derived *>(this);

        PteVal const old_pte = *pteval;
        PteVal const new_pte = derived->make_pteval(level, phys, flags);

        if (new_pte == old_pte) {
            return;
        }

        *pteval = new_pte;

        if (Derived::is_present(old_pte)) {
            synchroniser->append(virt, level, *pteval & X86_MMUF_GLOBAL, Derived::is_large_page_mapping(old_pte));
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::split_mapping(LevelType level, PteVal volatile *pte, VirtAddr virt_addr, 
                                       TravelContext *context, PageSynchroniser *synchroniser) -> Status {
        auto phys_table = alloc_page_table();
        if (!phys_table) {
            return Status::OutOfMem;
        }
        auto entry = reinterpret_cast<PteVal volatile *>(phys_to_virt(phys_table));
        auto const max_entries = Self::max_entries(level);
        auto const page_size = Self::page_size(level);

        // Extract arch MMU flags from pte.
        auto mmuflags = PagingTraits::arhc_mmu_flags_from_pte(level, *pte);
        // Extract physical address from pte.
        PhysAddr phys_addr = PagingTraits::phys_addr_from_pte(level, *pte);

        auto const lower_level = PagingTraits::next_level(level);
        for (auto i = 0; i < max_entries; ++i) {
            update_entry(entry + i, lower_level, phys_addr, virt_addr, mmuflags, synchroniser);
            phys_addr += page_size;
            virt_addr += page_size;
        }

        update_entry(pte, level, phys_table, virt_addr, mmuflags, synchroniser);
        return Status::Ok;
    }

    TEMPLATE FORCE_INLINE
    auto X86_PAGE_TABLE::make_mapping_context(MapContext *context, VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags)
        -> Status {
        auto const derived = static_cast<Derived *>(this);
        ustl::mem::construct_at(context, va, pa, n, flags, 4096);
        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::prepare_map_pages(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapContext *context)
        -> Status {
        auto const derived = static_cast<Derived *>(this);
        if (derived->check_virt_addr(va)) {
            return Status::InvalidArguments;
        }
        if (derived->is_flags_allowed(flags)) {
            return Status::InvalidArguments;
        }
        for (auto i = 0; i < len; ++i) {
            if (derived->check_phys_addr(pa[i])) {
                return Status::InvalidArguments;
            }
        }

        return make_mapping_context(context, va, pa, len, flags);
    }

    TEMPLATE
    auto X86_PAGE_TABLE::map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control, usize *mapped)
        -> Status {
        canary_.verify();

        if (!n) {
            return Status::Ok;
        }

        MapContext context;
        auto status = prepare_map_pages(va, &pa, n, flags, &context);
        if (status != Status::Ok) {
            return status;
        }

        PageSynchroniser synchroniser{};
        auto pgd = reinterpret_cast<PteVal *>(virt_);
        {
            ustl::sync::LockGuard guard(mutex_);
            status = create_mapping(Self::top_level(), pgd, &context, control, &synchroniser);

            if (status != Status::Ok) {
                // Failed to map, need we to unmap those mapped pages.
                TravelContext unmap_context{context.virt_cursor().consumed_range()};
                status = remove_mapping(Self::top_level(), pgd, &unmap_context, UnmapControl::None, &synchroniser);
            }
            synchroniser.sync();
        }
        if (status != Status::Ok) {
            return status;
        }

        if (mapped) {
            *mapped = context.num_mapped();
        }

        return status;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl control, usize *mapped)
        -> Status {
        canary_.verify();
        if (len == 0) {
            return Status::Ok;
        }

        MapContext context;
        auto status = prepare_map_pages(va, pa, len, flags, &context);
        if (status != Status::Ok) {
            return status;
        } 

        PageSynchroniser synchroniser{};
        auto pgd = reinterpret_cast<PteVal *>(virt_);
        {
            ustl::sync::LockGuard guard(mutex_);
            status = create_mapping(Self::top_level(), pgd, &context, control, &synchroniser);

            if (status != Status::Ok) {
                TravelContext unmap_context{context.virt_cursor().consumed_range()};
                status = remove_mapping(Self::top_level(), pgd, &unmap_context, UnmapControl::None, &synchroniser);
            }

            synchroniser.sync();
        }

        if (status != Status::Ok) {
            return status;
        }

        if (mapped) {
            *mapped = context.num_mapped();
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::create_mapping(LevelType level, ai_virt PteVal volatile *table, MapContext *context, 
                                        MapControl control, PageSynchroniser *synchroniser) -> Status {
        if (level == PagingTraits::kFinalLevel) {
            this->create_mapping_at_l0(table, context, control, synchroniser);
        }
        auto const derived = static_cast<Derived *>(this);
        auto const level_page_size = Self::page_size(level);
        auto const allow_large_page_mapping = bool(control & MapControl::TryLargePage);
        auto const large_page_supported = derived->level_can_be_terminal(LevelType(level));
        auto const max_entries = Self::max_entries(level);
        auto const mmuflags = context->flags();

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            ai_virt PteVal volatile *entry = table + index;

            if (!Derived::is_present(*table)) { // The entry do not exists.
                auto phys_addr = context->phys_addr();
                auto virt_addr = context->virt_addr();
                if (allow_large_page_mapping && large_page_supported) {
                    if (is_aligned(phys_addr, level_page_size) && 
                        is_aligned(virt_addr, level_page_size) && 
                        context->remaining_size() >= level_page_size) 
                    {
                        // The current address range supports a large mapping.
                        update_entry(entry, level, phys_addr, virt_addr, mmuflags, synchroniser);

                        context->consume(level_page_size);
                        // Large page mapping entry is leaf of page table, so we directly start to next iteration.
                        continue;
                    }
                }

                // The request do not cover the whole large page, so create a lower mappings to then map it.
                PhysAddr new_table = this->alloc_page_table();
                update_entry(entry, level, new_table, virt_addr, mmuflags, synchroniser);
            } else {  
                if (Derived::is_large_page_mapping(*table)) {
                    if (bool(control & MapControl::ErrorIfExisting)) {
                        // The entry exists and it is a large page mapping, if specifies options 
                        // MapControl::ErrorIfExisting then need we to report an error back to caller.
                        return Status::Error;
                    }

                    // As mush as possible not to overwrittring large page mapping.
                    // We directly skip this virtual address without the modification of physical address.
                    context->skip(level_page_size);
                    continue;
                }
            }

            auto status = create_mapping(PagingTraits::next_level(level),
                                  get_next_table_unchecked(*entry),
                                        context, control, synchroniser);
            if (Status::Ok != status) {
                return status;
            }
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::create_mapping_at_l0(PteVal volatile *table, MapContext *context, MapControl control, 
                                              PageSynchroniser *synchroniser) -> Status {
        CXX11_CONSTEXPR
        auto const level = PagingTraits::kFinalLevel;
        auto const readonly = (context->flags() & X86MmuFlags::PermMask) == X86MmuFlags::Writable;
        auto const page_size = Self::page_size(level);
        auto const max_entries = Self::max_entries(level);

        auto index = virt_to_index(PagingTraits::kFinalLevel, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            ai_phys PteVal volatile *entry = table + index;
            auto const is_existing = Derived::is_present(*table);

            if (is_existing) {
                if (!!(control & MapControl::ErrorIfExisting)) {
                    return Status::Error;
                } else if (!!(control & MapControl::SkipIfExisting)) {
                    context->skip(page_size);
                    continue;
                } else if (!!(control & MapControl::OverwriteIfExisting)) {
                    if (!readonly) {
                        unmap_entry(entry, level, context->virt_addr(), synchroniser);

                        // Before installing a new entry, the old entry invalidated must be
                        // flushed to make the changes visible. Otherwise the new entry may
                        // not work correctly.
                        synchroniser->sync();
                    }
                }
            }

            auto const [phys, virt] = context->take(page_size);
            update_entry(entry, level, phys, virt, context->flags(), synchroniser);
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::remove_mapping_at_l0(PteVal volatile *pte, TravelContext *context, 
                                              UnmapControl, PageSynchroniser *synchroniser)  -> Status {
        CXX11_CONSTEXPR
        auto const level = PagingTraits::kFinalLevel;
        auto const max_entries = Self::max_entries(level);
        auto const level_page_size = Self::page_size(level);

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() > 0; ++index) {
            PteVal volatile *entry = pte + index;
            if (Derived::is_present(*entry)) {
                unmap_entry(entry, level, context->virt_addr(), synchroniser);
            }

            context->consume(level_page_size);
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::remove_mapping(LevelType level, PteVal volatile *pte, TravelContext *context, 
                                        UnmapControl control, PageSynchroniser *synchroniser) -> Status {
        if (level == PagingTraits::kFinalLevel) {
            return remove_mapping_at_l0(pte, context, control, synchroniser);
        }

        auto const max_entries = Self::max_entries(level);
        auto const level_page_size = Self::page_size(level);

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            PteVal volatile *entry = pte + index;
            if (!Derived::is_present(*entry)) {
                // Skip those null entries.
                context->consume(level_page_size);
                continue;
            }

            VirtAddr const virt_addr = context->virt_addr();
            if (Derived::is_large_page_mapping(*entry)) {
                auto const va_aligned = align_down(virt_addr, level_page_size);
                if (va_aligned == virt_addr && context->size() >= level_page_size) {
                    // The request covers the entire large page, just unmap it.
                    unmap_entry(entry, level, virt_addr, synchroniser);
                    context->consume(level_page_size);
                    continue;
                }

                // The request does not cover the entire large page. Split it into multiple lower
                // level mappings to remove respectively.
                auto status = split_mapping(level, entry, va_aligned, context, synchroniser);
                if (Status::Ok != status) {
                    return status;
                }
            }

            auto table = get_next_table_unchecked(*entry);
            auto status = remove_mapping(PagingTraits::next_level(level), table, context, control, synchroniser);
            if (Status::Ok != status) {
                return status;
            }
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::unmap_pages(VirtAddr va, usize n, UnmapControl control, usize *unmapped) -> Status {  
        canary_.verify();

        if (!n) {
            return Status::Ok;
        }

        auto derived = static_cast<Derived *>(this);
        if (!derived->check_virt_addr(va)) {
            return Status::InvalidArguments;
        }

        PageSynchroniser synchroniser{};
        TravelContext context{virt_, n, PAGE_SIZE};
        
        Status status;
        auto pgd = reinterpret_cast<PteVal volatile *>(virt_);
        {
            ustl::sync::LockGuard guard(mutex_);
            status = remove_mapping(Self::top_level(), pgd, &context, control, &synchroniser);
            synchroniser.sync();
        }
        if (unmapped) {
            *unmapped = context.num_travelled();
        }

        return status;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::update_mapping_at_l0(PteVal volatile *pte, MapContext *context, 
                                              PageSynchroniser *synchroniser) -> Status {
        CXX11_CONSTEXPR
        auto const level = PagingTraits::kFinalLevel;
        auto const mmuflags = context->flags();
        auto const max_entries = Self::max_entries(level);
        auto const level_page_size = Self::page_size(PagingTraits::kFinalLevel);

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            PteVal volatile *entry = pte + index;
            if (Derived::is_present(*entry)) {
                PhysAddr const phys = PagingTraits::phys_addr_from_pte(level, *entry);
                update_entry(entry, level, phys, context->virt_addr(), mmuflags, synchroniser);
            } 

            context->consume(level_page_size);
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::update_mapping(LevelType level, PteVal volatile *pte, MapContext *context,
                                        PageSynchroniser *synchroniser) -> Status {
        if (level == PagingTraits::kFinalLevel) {
            return update_mapping_at_l0(pte, context, synchroniser);
        }

        auto const mmuflags = context->flags();
        auto const max_entries = Self::max_entries(level);
        auto const level_page_size = Self::page_size(level);

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            PteVal volatile *entry = pte + index;
            if (!Derived::is_present(*entry)) {
                context->consume(level_page_size);
                continue;
            } 

            auto virt = context->virt_addr();
            if (Derived::is_large_page_mapping(*entry)) {
                if (is_aligned(virt, level_page_size) && context->remaining_size() >= level_page_size) {
                    PhysAddr const phys = PagingTraits::phys_addr_from_pte(level, *entry);

                    // The request covers the entire page, just let us to update it.
                    update_entry(entry, level, phys, virt, mmuflags, synchroniser);
                    context->consume(level_page_size);
                    continue;
                }
            }

            auto status = update_mapping(PagingTraits::next_level(level), get_next_table_unchecked(*entry), context, synchroniser);
            if (Status::Ok != status) {
                return status;
            }
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status {
        canary_.verify();
        if (!n) {
            return Status::Ok;
        }

        auto derived = static_cast<Derived *>(this);
        if (!derived->check_virt_addr(va)) {
            return Status::InvalidArguments;
        }

        PageSynchroniser synchroniser{};
        MapContext context{va, 0, n, flags, PAGE_SIZE};
        Status status{};
        auto pgd = reinterpret_cast<PteVal volatile *>(virt_);
        {
            ustl::sync::LockGuard guard(mutex_);
            status = update_mapping(Self::top_level(), pgd, &context, &synchroniser);
        }

        return status;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::read_mapping(PteVal volatile *pte, VirtAddr va, LevelType *out_level, PteVal *out_pte) 
        -> Status {
        auto level = Self::top_level();
        while (1) {
            PteVal volatile *entry = pte + virt_to_index(level, va);
            if (!Derived::is_present(*entry)) {
                return Status::NotFound;
            } else if (Derived::is_large_page_mapping(*entry)) {
                if (out_level) {
                    *out_level = level;
                }
                if (out_pte) {
                    *out_pte = *entry;
                }

                return Status::Ok;
            }

            if (level == PagingTraits::kFinalLevel) {
                return Status::NotFound;
            }
            level = PagingTraits::next_level(level);
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status {  
        canary_.verify();

        PteVal pte = 0;
        LevelType level = Self::top_level();
        Status status;
        {
            ustl::sync::LockGuard guard(mutex_);
            status = read_mapping(reinterpret_cast<PteVal volatile *>(virt_), va, &level, &pte);
        }

        if (Status::Ok != status) {
            return status;
        }
        if (pa) {
            *pa = PagingTraits::phys_addr_from_pte(level, pte);
        }
        if (flags) {
            *flags = mmuflags_cast<X86MmuFlags>(X86MmuFlags(pte));
        }

        return Status::Ok;
    }

    TEMPLATE
    auto X86_PAGE_TABLE::harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status
    {  return Status::Unimplemented;  }

    TEMPLATE
    auto X86_PAGE_TABLE::alias_to(IX86PageTable const &other, VirtAddr base, usize nr_pages) -> Status
    {  return Status::Unimplemented;  }

} // namespace arch::paging

#undef X86_PAGE_TABLE
#undef TEMPLATE