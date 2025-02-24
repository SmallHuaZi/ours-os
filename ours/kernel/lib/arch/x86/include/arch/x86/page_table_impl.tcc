#include "ustl/result.hpp"
#ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP
#   include <arch/x86/page_table_impl.hpp>
#endif // #ifndef ARCH_X86_PAGE_TABLE_IMPL_HPP

#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/sync/lockguard.hpp>

#define TEMPLATE \
    template <typename Derived, typename PagingOptions>

#define X86_PAGE_TABLE \
    X86PageTableImpl<Derived, PagingOptions>

namespace arch::x86 {
    using ustl::mem::is_aligned;

    TEMPLATE FORCE_INLINE
    auto X86_PAGE_TABLE::make_mapping_context(MappingContext *context, VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags) -> Status
    {
        auto const derived = static_cast<Derived *>(this);
        ustl::mem::construct_at(context, va, &pa, n, flags, 4096);
    }

    TEMPLATE FORCE_INLINE
    auto X86_PAGE_TABLE::prepare_map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MappingContext *context) -> Status
    {
        auto const derived = static_cast<Derived *>(this);
        if (derived->check_virt_addr(va)) {
            return Status::InvalidArguments;
        }
        if (derived->check_phys_addr(pa)) {
            return Status::InvalidArguments;
        }
        if (derived->is_flags_allowed(flags)) {
            return Status::InvalidArguments;
        }

        return make_mapping_context(va, &pa, 1, flags, context);
    }

    TEMPLATE
    auto X86_PAGE_TABLE::map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control) 
        -> ustl::Result<usize, Status>
    {
        canary_.verify();

        MappingContext context;
        auto status = prepare_map_pages(va, pa, n, flags, &context);
        if (status != Status::Ok) {
            return ustl::err(status);
        } else if (n == 0) {
            return ustl::ok<usize>(0);
        }

        {
            ustl::sync::LockGuard guard(mutex_);
            status = create_mapping(Self::top_level(), virt_, &context, control);
        }

        if (status != Status::Ok) {
            return ustl::err(status);
        }
        return ustl::ok<usize>(context.mapped());
    }

    TEMPLATE
    auto X86_PAGE_TABLE::prepare_map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MappingContext *context) -> Status
    {
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
    auto X86_PAGE_TABLE::map_pages_bulk(VirtAddr va, PhysAddr *pa, usize len, MmuFlags flags, MapControl control)
        -> ustl::Result<usize, Status>
    {
        canary_.verify();

        MappingContext context;
        auto status = prepare_map_pages_bulk(va, pa, len, flags, &context);
        if (status != Status::Ok) {
            return ustl::err(status);
        } else if (len == 0) {
            return ustl::ok<usize>(0);
        }

        {
            ustl::sync::LockGuard guard(mutex_);
            status = create_mapping(Self::top_level(), virt_, &context, control);
        }

        if (status != Status::Ok) {
            return ustl::err(status);
        }
        return ustl::ok<usize>(context.mapped());
    }

    TEMPLATE
    auto X86_PAGE_TABLE::create_mapping(usize level, ai_virt Pte volatile *table, MappingContext *context, MapControl control) 
        -> Status
    {
        if (level == 0) {
            this->create_mapping_at_l0(table, context, control);
        }
        auto const derived = static_cast<Derived *>(this);
        auto const level_page_size = Self::page_size(level);
        auto const allow_large_page_mapping = bool(control & MapControl::TryLargePage);
        auto const large_page_supported = derived->has_supported_page_size(level);
        auto const max_entries = Self::max_entries(level);

        auto index = virt_to_index(level, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            ai_virt Pte volatile *entry = table + index;

            if (!Derived::is_present(*table)) { // The entry do not exists.
                if (allow_large_page_mapping && large_page_supported) {
                    auto const pa_is_aligned = is_aligned(context->phys_addr(), level_page_size);
                    auto const va_is_aligned = is_aligned(context->virt_addr(), level_page_size);
                    if (pa_is_aligned && va_is_aligned && context->remaining_size() >= level_page_size) {
                        // The current address range supports a large mapping.
                        update_entry(entry, context->phys_addr(), context->virt_addr(), context->flags());

                        context->consume(level_page_size);
                        // Large page mapping entry is leaf of page table, so we directly start to next iteration.
                        continue;
                    }
                }

                PhysAddr new_table = this->alloc_page_table(); 
                update_entry(entry, new_table, 0, context->flags());
            } else {  // The entry exists.
                if (bool(control & MapControl::ErrorIfExisting)) {
                    return Status::Error;
                }

                if (Derived::is_large_page_mapping(*table)) {
                    // As possible as overwrittring large page mapping. 
                    // We directly skip this virtual address without the modification of physical address.
                    context->skip(level_page_size);
                    continue;
                }
            }

            this->create_mapping(level - 1, derived->get_next_table_unchecked(entry), context, control);
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::create_mapping_at_l0(Pte volatile *table, MappingContext *context, MapControl control) -> Status
    {
        auto const readonly = (context->flags() & MmuFlags::PermMask) == MmuFlags::Writable;
        auto const page_size = Self::page_size(0);
        auto const max_entries = Self::max_entries(0);

        auto index = virt_to_index(0, context->virt_addr());
        for (; index < max_entries && context->size() != 0; ++index) {
            ai_phys Pte volatile *entry = table + index;
            auto const is_existing = Derived::is_present(*table);

            if (is_existing) {
                if (bool(control & MapControl::ErrorIfExisting)) {
                    return Status::Error;
                } else if (bool(control & MapControl::SkipIfExisting)) {
                    context->skip(page_size);
                    continue;
                } else if (bool(control & MapControl::OverwriteIfExisting)) {
                    if (!readonly) {
                        unmap_entry(entry, 0);
                        context->synchroniser().sync();
                    }
                }
            }

            auto const [phys, virt] = context->take(page_size);
            update_entry(entry, phys, virt, context->flags());
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::update_entry(ai_virt Pte volatile *pte, PhysAddr phys, VirtAddr virt, MmuFlags flags) -> void 
    {
        auto const derived = static_cast<Derived *>(this);

        Pte const old_pte = *pte;
        Pte const new_pte = derived->make_pte(phys, flags);

        if (new_pte == old_pte) {
            return; 
        }

        *pte = new_pte;

        if (Derived::is_present(old_pte)) {
        }
    }

    TEMPLATE
    auto X86_PAGE_TABLE::unmap_pages(VirtAddr va, usize n) -> Status
    {}

    TEMPLATE
    auto X86_PAGE_TABLE::protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status
    {}

    TEMPLATE
    auto X86_PAGE_TABLE::query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status
    {}

    TEMPLATE
    auto X86_PAGE_TABLE::harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status
    {}

} // namespace arch::x86

#undef X86_PAGE_TABLE
#undef TEMPLATE