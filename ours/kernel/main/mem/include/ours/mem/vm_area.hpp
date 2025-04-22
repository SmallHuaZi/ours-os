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
    /// `VmArea` is a representation of a contiguous range of virtual memory space.
    class VmArea: public VmAreaBase {
        typedef VmArea      Self;
        typedef VmAreaBase  Base;
    public:
        static auto create(ustl::Rc<VmAspace>, VirtAddr, usize, VmaFlags, char const *) 
            -> ustl::Result<ustl::Rc<Self>, Status>;

        auto activate() -> void;

        auto destroy() -> void;

        /// Create a mapping unit in page range [vma_off, vma_off + nr_pages).
        auto create_mapping(PgOff vma_off, usize nr_pages, VmaFlags, PgOff vmo_off, MmuFlags, ustl::Rc<VmObject>, char const *name) 
            -> ustl::Result<ustl::Rc<VmMapping>, Status>;

        /// Create a sub-VMA in page range [vma_off, vma_off + nr_pages).
        auto create_subvma(PgOff vma_off, usize nr_pages, VmaFlags, char const *name) 
            -> ustl::Result<ustl::Rc<VmArea>, Status>;

        auto unmap(usize base, usize size) -> void;

        auto protect(usize base, usize size, MmuFlags flags) -> void;

        auto contains(VirtAddr addr) const -> bool;

        auto address_range() const -> gktl::Range<VirtAddr>;

        FORCE_INLINE
        auto num_pages() const -> usize {
            return size_ / PAGE_SIZE;
        }

        FORCE_INLINE
        auto start_vpn() const -> Vpn {
            return base_ / PAGE_SIZE;
        }

        FORCE_INLINE
        auto end_vpn() const -> Vpn {
            return (base_ + size_) / PAGE_SIZE;
        }

        // On logic, it should be protected to avoid the incorrect use. But 
        VmArea(ustl::Rc<VmAspace>, VirtAddr, usize, VmaFlags, char const *);
    protected:
        auto prepare_create_subvma(PgOff vma_off, usize nr_pages, VmaFlags &, 
                                   VirtAddr ai_out &base, VirtAddr ai_out &size) const -> bool;

        /// Check whether a given `|mmflags|` is valid. If valid return a set of permission in VmaFlags,
        /// otherwise a `VmaFlags::None`
        auto check_mmuflags(MmuFlags mmflags) const -> VmaFlags;

    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmRootArea;
        friend class VmMappingHandler;

        VmAreaSet subvmas_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP