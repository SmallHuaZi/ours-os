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

#include <arch/paging/options.hpp>
#include <arch/paging/page_table_ept.hpp>
#include <arch/paging/page_table_mmu.hpp>
#include <arch/paging/x86_pagings.hpp>
#include <arch/options.hpp>

#include <gktl/canary.hpp>
#include <ustl/option.hpp>
#include <ustl/placeholders.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/util/pack_options.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/traits/function_detector.hpp>
#include <ustl/views/span.hpp>

namespace arch {
namespace paging {
    USTL_MPL_CREATE_METHOD_DETECTOR(Init, init);
    USTL_TYPE_OPTION(PhysToVirt, PhysToVirt);

    /// `PageTable` is a high-level abstract to virutal memory mapping.
    /// `|Mutex|` should provides the interface `lock` and `unlock` at least.
    /// `|PageManager|`  should provides the interface `alloc_page` and `free_page`  at least.
    template <typename Options>
    class X86PageTable {
      public:
        using Mmu = paging::X86PageTableMmu<Options>;
        using Ept = paging::X86PageTableEpt<Options>;
        using PagingTraits = typename Mmu::PagingTraits;

        template <typename Derived>
        CXX11_CONSTEXPR
        auto init(PhysAddr pgd_pa, VirtAddr pgd_va) -> Status;

        template <typename Derived>
        CXX11_CONSTEXPR
        auto init() -> Status;

        template <typename... Args> 
        FORCE_INLINE CXX11_CONSTEXPR
        auto init_ept(Args... args) -> Status {
            return init<Ept>(args...);
        }

        template <typename... Args> 
        FORCE_INLINE CXX11_CONSTEXPR
        auto init_mmu(Args... args) -> Status {
            return init<Mmu>(args...);
        }

        /// Sees IX86PageTable::map_pages.
        FORCE_INLINE
        auto map_pages(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl action, usize *mapped) 
            -> Status {
            return pimpl_->map_pages(va, pa, n, flags, action, mapped);
        }

        /// Sees IX86PageTable::map_pages.
        FORCE_INLINE
        auto map_pages_bulk(VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags, MapControl action, usize *mapped) 
            -> Status {
            return pimpl_->map_pages_bulk(va, pa, n, flags, action, mapped);
        }

        /// Sees IX86PageTable::unmap_pages.
        FORCE_INLINE
        auto unmap_pages(VirtAddr va, usize n, UnmapControl control, usize *unmapped) -> Status {
            return pimpl_->unmap_pages(va, n, control, unmapped);
        }

        /// Sees IX86PageTable::protect_pages.
        FORCE_INLINE
        auto protect_pages(VirtAddr va, usize n, MmuFlags flags) -> Status {
            return pimpl_->protect_pages(va, n, flags);
        }

        /// Sees IX86PageTable::query_mapping.
        FORCE_INLINE
        auto query_mapping(VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status {
            return pimpl_->query_mapping(va, pa, flags);
        }

        /// Sees IX86PageTable::harvest_accessed.
        FORCE_INLINE
        auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status {
            return pimpl_->harvest_accessed(va, n, action);
        }

        FORCE_INLINE
        auto alias_to(X86PageTable const &other, VirtAddr base, usize nr_pages) -> Status {
            return pimpl_->alias_to(*other.pimpl_, base, nr_pages);
        }

        FORCE_INLINE
        auto pgd_phys() const -> PhysAddr {
            return pimpl_->pgd_phys();
        }

        FORCE_INLINE
        auto install() const -> void {
            pimpl_->install();
        }
      private: 
        union Storage {
            alignas(Mmu) char mmu_[sizeof(Mmu)];
            alignas(Ept) char ept_[sizeof(Ept)];
        } storage_;
        paging::IX86PageTable *pimpl_;
    };

    template <typename Options>
    template <typename Derived>
    FORCE_INLINE CXX11_CONSTEXPR
    auto X86PageTable<Options>::init(PhysAddr pgd_pa, VirtAddr pgd_va) -> Status {
        static_assert(traits::IsBaseOfV<IX86PageTable, Derived>, "You don't know?");

        Derived *derived = new (&storage_) Derived();
        auto status = derived->init(pgd_pa, pgd_va);
        if (status != Status::Ok) {
            return status;
        }
        pimpl_ = derived;

        return Status::Ok;
    }

    template <typename Options>
    template <typename Derived>
    FORCE_INLINE CXX11_CONSTEXPR
    auto X86PageTable<Options>::init() -> Status {
        // Try allocate page from PageSource
        static_assert(traits::IsBaseOfV<IX86PageTable, Derived>, "You don't know?");

        Derived *derived = new (&storage_) Derived();
        auto status = derived->init();
        if (status != Status::Ok) {
            return status;
        }
        pimpl_ = derived;

        return Status::Ok;
    }

    struct PageTableDefaultOptions {
        CXX11_CONSTEXPR
        static auto const kPagingLevel = X86PagingLevel::PageMapLevel4;

        typedef ustl::sync::Mutex Mutex;

        /// We provide a identity mapping converter as default.
        struct PhysToVirt {
            FORCE_INLINE CXX11_CONSTEXPR
            auto phys_to_virt(PhysAddr addr) -> VirtAddr {
                OX_PANIC("Nerver call this {}", __func__);
            }
        };

        /// The example code which nerver be invoked. 
        struct PageAllocator {
            auto alloc_pages(usize nr_pages, usize alignment) -> PhysAddr {
                OX_PANIC("Nerver call this {}", __func__);
            }

            auto free_pages(PhysAddr page, usize nr_pages) -> void {
                OX_PANIC("Nerver call this {}", __func__);
            }
        };

        typedef void TlbInvalidator;
    };

    template <typename... Options>
    struct MakeX86PageTable {
        typedef typename ustl::PackOptions<PageTableDefaultOptions, Options...>::Type PackedOptions;
        typedef X86PageTable<PackedOptions> Type;
    };
} // namespace paging

    /// The `PageTable` type alias follows standard naming conventions for generic page table implementations.
    /// It supports the following template parameters:
    ///     1) \c `MutexT<>`       - Locking mechanism for concurrent access
    ///     2) \c `PageSourceT<>`  - Physical page allocation strategy
    ///     3) \c `PageFlusherT<>` - TLB and cache shootdown handling
    ///     4) \c `PagingLevelV<>` - Paging config
    ///     5) \c `X86PageExt<>`   - Whether enable x86 page extension. It is only valid on 32-bits platform.
    ///
    /// These parameters allow customization of:
    ///   - Synchronization primitives
    ///   - Memory management policies
    ///   - Cache coherence protocols
    template <typename... Options>
    using PageTable = typename paging::MakeX86PageTable<Options...>::Type;

} // namespace arch

#endif // #ifndef ARCH_X86_PAGE_TABLE_HPP