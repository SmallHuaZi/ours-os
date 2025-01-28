#include <ours/arch/x86/page_table.hpp>
#include <ours/mem/mod.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/mapping_context.hpp>

#include <ours/assert.hpp>

#include <arch/x86/tlb.hpp>
#include <arch/x86/cache.hpp>

#include <ustl/sync/lockguard.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    static Pte KERNEL_PAGE_TABLE_ENTRIES[X86PagingConfig::PAGE_LEVEL][X86PagingConfig::MAX_NR_PTE];

    template
    class X86PageTable<X86PagingConfig>;

    template <>
    auto down_cast<X86MmuFlags>(MmuFlags flags) -> X86MmuFlags
    {
        X86MmuFlags arch_flags{};
        if (!(flags & MmuFlags::Executable)) {
            arch_flags |= X86MmuFlags::NonExecutable;
        }
        if (flags & MmuFlags::Writable) {
            arch_flags |= X86MmuFlags::Writable;
        }
        if (flags & MmuFlags::User) {
            arch_flags |= X86MmuFlags::User;
        }

        if (!(flags & MmuFlags::Cached)) {
            arch_flags |= X86MmuFlags::Discache;
        }

        return arch_flags;
    }

    // Normal page table.
    template <>
    auto X86PageTable<X86PagingConfig>::
    refresh(Pte *pte) -> void
    {
        arch::Cache::flush(pte->address());
        arch::Tlb::flush(pte->address());
    }

#ifdef OURS_CONFIG_X86_ENABLE_EPT
    template
    class X86PageTable<X86ExtentPagingConfig>;

    template <>
    auto down_cast<X86EptFlags>(MmuFlags flags) -> X86EptFlags
    {}

    // Vm page table.
    template <>
    auto X86PageTable<X86ExtentPagingConfig>::
    refresh(Pte volatile *) -> void
    {}
#endif

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    map_pages(VirtAddr virt_addr, PhysAddr phys_addr, usize n, MmuFlags flags) -> Status 
    {
        canary_.verify();

        MappingContext context{ virt_addr, phys_addr, n, flags };
        {
            USTL_LOCK_GUARD(mutex_) lock_guard{mutex_};
            auto res = create_mapping(table_virt_addr_, &context, Self::level());
        }
        context.finish();
        
        return Status::Ok;
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    create_mapping_at_level_0(Pte *table, MappingContext *context) -> Status
    {
        MmuFlags const flags = context->flags();
        usize const page_size = PagingConfig::page_size(0);
        usize index = PagingConfig::index_of(context->virt_addr(), 0);

        for (; index <= PagingConfig::MAX_NR_PTE; ++index) {
            auto entry = table + index;

            // The entry exists and this mapping is just to update the flags.
            if (entry->is_present()) {
                if (entry->address() != context->phys_addr()) {
                    unmap_entry(entry, context);
                } else if ((flags & MmuFlags::PermissionsMask) != MmuFlags::Readable) {
                    // The entry exists and this mapping is just to update the flags.
                    update_entry(entry, context->phys_addr(), context->virt_addr(), flags);
                }
            } else {
                update_entry(entry, context->phys_addr(), context->virt_addr(), flags);
            }

            context->consume(page_size);
        }

        return Status::Ok;
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    create_mapping(Pte *table, MappingContext *context, usize level) -> Status
    {
        // Basic case, mapping a page alone.
        if (0 == level) {
            return this->create_mapping_at_level_0(table, context);
        }

        usize const page_size = PagingConfig::page_size(level);
        usize index = PagingConfig::index_of(context->virt_addr(), level);
        for (; index <= PagingConfig::MAX_NR_PTE; ++index) {
            auto entry = table + index;

            // The entry doesn't exists, so we need to create a new entry.
            if (!entry->is_present()) {
                this->populate_entry(entry, context);
            }

            // It exists but not upgrades.
            auto ret = this->create_mapping(entry, context, level - 1);
        }       
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    populate_entry(Pte *entry, MappingContext *context) -> Status
    {
        if (auto option = alloc_frames(Gaf::Mmu)) {
            auto frame = *option;
            auto status = this->update_entry(entry, frame->address(), {}, context->flags());
            if (Status::Ok == status) {
                pages_ += 1;
                return Status::Ok;
            }
        }

        return Status::Fail;
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    unmap_pages(VirtAddr virt_addr, usize n) -> Status
    {
        canary_.verify();
        MappingContext context{ virt_addr, 0, n, {} };
        {
            USTL_LOCK_GUARD(mutex_) lock_guard{mutex_};
            auto res = remove_mapping(table_virt_addr_, &context, Self::level());
        }
        context.finish();
        
        return Status::Ok;
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    remove_mapping_at_level_0(Pte *table, MappingContext *context) -> Status
    {
        MmuFlags const flags = context->flags();
        usize const page_size = PagingConfig::page_size(0);
        usize index = PagingConfig::index_of(context->virt_addr(), 0);

        for (; index <= PagingConfig::MAX_NR_PTE; ++index) {
            auto entry = table + index;

            // The entry exists and this mapping is just to update the flags.
            if (!entry->is_present()) {
                unmap_entry(entry, context);
            }

            context->consume(page_size);
        }

        return Status::Ok;
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    remove_mapping(Pte *table, MappingContext *context, usize level) -> Status
    {
        // Basic case, mapping a page alone.
        if (0 == level) {
            return this->create_mapping_at_level_0(table, context);
        }

        usize const page_size = PagingConfig::page_size(level);
        usize index = PagingConfig::index_of(context->virt_addr(), level);
        for (; index <= PagingConfig::MAX_NR_PTE; ++index) {
            auto entry = table + index;

            // The entry doesn't exists, so we need to create a new entry.
            if (!entry->is_present()) {
                context->skip(page_size);
                continue;
            }

            // It exists but not upgrades.
            auto ret = this->remove_mapping(entry, context, level - 1);
        }       
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    protect_pages(VirtAddr virt_addr, usize n, MmuFlags flags) -> Status
    {
        canary_.verify();
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    query_mapping(VirtAddr virt_addr) -> ustl::Option<Pte>
    {
        canary_.verify();
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    update_entry(Pte *entry, PhysAddr phys_addr, VirtAddr virt_addr, MmuFlags flags) -> Status 
    {
        Pte old_pte = *entry;

        auto const arch_mmu_flags = down_cast<ArchMmuFlags>(flags);
        Pte new_pte = Pte::create(phys_addr, arch_mmu_flags | X86MmuFlags::Present);

        *entry = new_pte;
        this->refresh(entry);
    }

    template <typename PagingConfig>
    auto X86PageTable<PagingConfig>::
    unmap_entry(Pte *entry, MappingContext *context) -> Status
    {
        Pte old_pte = *entry;
        auto frame = phys_to_frame(entry->address()).value();
        context->to_free_.push_back(*frame);

        *entry = 0;
        this->refresh(entry);
    }

} // namespace ours::mem