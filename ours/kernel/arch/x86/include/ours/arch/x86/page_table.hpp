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

#ifndef OURS_ARCH_X86_PAGE_TABLE_HPP
#define OURS_ARCH_X86_PAGE_TABLE_HPP 1

#include <ours/mem/mmu_flags.hpp>
#include <ours/mem/details/mapping_context.hpp>
#include <ours/arch/x86/mmu_paging_config.hpp>

#include <ours/status.hpp>

#ifdef OURS_CONFIG_X86_ENABLE_EPT
#include <ours/arch/x86/ept_paging_config.hpp>
#endif

#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/views/span.hpp>
#include <ustl/collections/array.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    struct alignas(usize) Pte
    {
        typedef Pte Self;
        typedef ustl::traits::UnderlyingTypeT<X86MmuFlags>  PteVal;

        Pte(): _0(0) {}
        Pte(PteVal inner): _0(inner) {}

        ~Pte() = default;

        static auto create(PhysAddr addr, X86MmuFlags flags) -> Pte
        {  return Pte(addr | static_cast<PteVal>(flags));  }

        auto address() const -> PhysAddr
        {  return _0 & ~static_cast<PteVal>(X86MmuFlags::Mask);  }

        auto flags() const -> X86MmuFlags
        {  return X86MmuFlags(_0 & static_cast<PteVal>(X86MmuFlags::Mask));  }

        auto test(X86MmuFlags flags) const -> bool
        {  return bool(_0 & static_cast<PteVal>(flags));  }

        auto is_present() const -> bool
        {  return this->test(X86MmuFlags::Present);  }

        auto operator=(Self const &other) -> Self volatile &
        {
            _0 = other._0;
            return *this;
        }

        usize volatile _0;
    };

    inline auto operator&(Pte pte, X86MmuFlags flags) -> bool
    {
        typedef ustl::traits::UnderlyingTypeT<X86MmuFlags>  Type;
        return pte._0 & static_cast<Type>(flags);
    }

    inline auto operator|(Pte pte, X86MmuFlags flags) -> Pte
    {
        typedef ustl::traits::UnderlyingTypeT<X86MmuFlags>  Type;
        return Pte(pte._0 & static_cast<Type>(flags));
    }

    enum class X86PtFlags: usize {
        Shared,
        Independent,

        User = (1 << 4),
        Ept = (1 << 7),
    };
    USTL_ENABLE_ENUM_BITS(X86PtFlags);

    class IX86PageTable
    {
        typedef IX86PageTable   Self;
    public:
        IX86PageTable() = default;
        virtual ~IX86PageTable() = default;

        /// Maps `n` frames of physical memory starting from `phys_addr` to the virtual address `virt_addr`,
        /// and the mapping is done with the specified `flags`.
        virtual auto map_pages(VirtAddr virt_addr, PhysAddr phys_addr, usize n, MmuFlags flags) 
            -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto unmap_pages(VirtAddr virt_addr, usize n) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto protect_pages(VirtAddr virt_addr, usize n, MmuFlags flags) -> Status = 0;

        /// Unmaps `n` pages of virtual memory starting from the address `virt_addr`.
        virtual auto query_mapping(VirtAddr virt_addr) -> ustl::Option<Pte> = 0;

        auto page_usage() const -> usize
        {  return pages_.load();  }

    protected:
        X86PtFlags flags_;
        Pte *table_phys_addr_;
        Pte *table_virt_addr_;

        ustl::sync::Mutex       mutex_;
        ustl::sync::AtomicUsize pages_;
        ustl::sync::AtomicUsize refcnt_;
    };

    /// Class `X86PageTable` is a high-level implementation of architecturelly page table.
    template <typename PagingConfig>
    class X86PageTable
        : public IX86PageTable
    {
        typedef X86PageTable      Self;
        typedef typename PagingConfig::ArchMmuFlags     ArchMmuFlags;
    public:
        /// Sees X86PageTableInterface::map_pages.
        virtual auto map_pages(VirtAddr, PhysAddr, usize, MmuFlags) -> Status override;

        /// Sees X86PageTableInterface::unmap_pages.
        virtual auto unmap_pages(VirtAddr, usize) -> Status override;

        /// Sees X86PageTableInterface::protect_pages.
        virtual auto protect_pages(VirtAddr, usize, MmuFlags) -> Status override;

        /// Sees X86PageTableInterface::query_mapping.
        virtual auto query_mapping(VirtAddr) -> ustl::Option<Pte> override;

        static auto level() -> usize
        {  return PagingConfig::PageLevel;  }

    private:
        /// Create a page table entry.
        auto create_mapping(Pte *, MappingContext *, usize) -> Status;
        auto create_mapping_at_level_0(Pte *, MappingContext *) -> Status;

        /// Remove a page table entry.
        auto remove_mapping(Pte *, MappingContext *, usize) -> Status;
        auto remove_mapping_at_level_0(Pte *, MappingContext *) -> Status;

        /// Update a page table entry.
        auto update_mapping(Pte *, MappingContext *, usize) -> Status;
        auto update_mapping_at_level_0(Pte *, MappingContext *) -> Status;

        /// Update a entry.
        auto update_entry(Pte *, PhysAddr, VirtAddr, MmuFlags) -> Status;

        auto unmap_entry(Pte *, MappingContext *) -> Status;

        auto populate_entry(Pte *, MappingContext *) -> Status;

        auto refresh(Pte *) -> void;

    private:
        GKTL_CANARY(X86PageTable, canary_);
    };

    typedef X86PageTable<X86PagingConfig>  PageTable;

#ifdef OURS_CONFIG_X86_ENABLE_EPT
    typedef X86PageTable<X86ExtentPagingConfig>   ExtentPageTable;
#endif // #ifdef OURS_CONFIG_X86_ENABLE_EPT

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_X86_PAGE_TABLE_H