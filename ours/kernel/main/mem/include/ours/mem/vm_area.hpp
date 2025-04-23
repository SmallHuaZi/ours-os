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
#ifndef OURS_MEM_VM_AREA_HPP
#define OURS_MEM_VM_AREA_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/vm_fault.hpp>
#include <ours/mem/vm_mapping.hpp>

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmaFlags: u32 {
        // For semantics, no any actual effects.
        None,

        // Only serve for VmMapping
        Write = BIT(1),
        Read  = BIT(2),
        Exec  = BIT(3),
        Share = BIT(4),
        PermMask = Write | Read | Exec | Share,

        // States 
        Active   = 0x10000,
        Pinned   = 0x20000,

        // Features.
        Anonymous = 0x40000,
        Mergeable = 0x80000,
    };
    USTL_ENABLE_ENUM_BITMASK(VmaFlags);

    /// `VmArea` is a representation of a contiguous range of virtual memory space.
    class VmArea: public ustl::RefCounter<VmArea> {
        typedef VmArea      Self;
    public:
        static auto create(ustl::Rc<VmAspace>, VirtAddr, usize, VmaFlags, char const *, ustl::Rc<Self> *) -> Status;

        /// Create a mapping unit in page range [vma_off, vma_off + nr_pages).
        auto create_mapping(PgOff vma_off, usize nr_pages, PgOff vmo_off, MmuFlags, ustl::Rc<VmObject>, 
                            char const *name, ustl::Rc<VmMapping> *) -> Status;

        /// Create a sub-VMA in page range [vma_off, vma_off + nr_pages).
        auto create_subvma(PgOff vma_off, usize nr_pages, VmaFlags, char const *name, ustl::Rc<VmArea> *) -> Status;

        /// Create a sub-VMA in page range [vma_off, vma_off + nr_pages).
        auto create_subvma(usize nr_pages, VmaFlags, char const *name, ustl::Rc<VmArea> *) -> Status;

        auto unmap(PgOff vma_off, usize nr_pages) -> void;

        /// Update rights of the subset of the area. 
        auto protect(PgOff vma_off, usize nr_pages, MmuFlags flags) -> void;

        auto contains(VirtAddr addr) const -> bool;

        // On logic, it should be protected to avoid the incorrect use. But 
        VmArea(ustl::Rc<VmAspace>, VirtAddr, usize, VmaFlags, char const *);
    protected:
        auto prepare_subrange(PgOff vma_off, usize nr_pages, VirtAddr ai_out &base, VirtAddr ai_out &size) 
            const -> bool;

        /// Check whether a given `|mmflags|` is valid. True if valid, otherwise invalid.
        auto validate_mmuflags(MmuFlags mmflags) const -> bool;

        auto activate() -> void;

        auto destroy() -> void;

        FORCE_INLINE
        auto has_range(VirtAddr base, usize size) const -> bool;

        /// This for the collections to compare two VMA.
        FORCE_INLINE CXX23_STATIC
        auto operator()(Self const &x, Self const &y) -> bool {
            return x.base_ < y.base_;
        }
    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmMapping;

        GKTL_CANARY(VmArea, canary_);
        char const *name_;
        VirtAddr  base_;
        VirtAddr  size_;
        VmaFlags  vmaf_;
        VmArea   *parent_;      // Null if it is root or dead.
        VmMapping *mapping_;    // Non-null if it is a leaf.
        ustl::Rc<VmAspace> aspace_;
        ustl::collections::intrusive::SetMemberHook<> subvma_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, subvma_hook_, ManagedOptions);
        USTL_DECLARE_MULTISET(Self, VmaSet, ManagedOptions);
        VmaSet subvmas_;
    };
} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP