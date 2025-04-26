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
#include <ours/mem/vm_area_or_mapping.hpp>

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/option.hpp>
#include <ustl/collections/intrusive/set.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    /// `VmArea` is a representation of a contiguous range of virtual memory space.
    class VmArea: public VmAreaOrMapping {
        typedef VmArea          Self;
        typedef VmAreaOrMapping Base;
    public:
        static auto create(VirtAddr, usize, VmaFlags, VmArea *, VmAspace *, char const *, ustl::Rc<Self> *) 
            -> Status;

        /// Create a mapping unit in page range [vma_off, vma_off + size).
        auto create_mapping(usize vma_ofs, usize size, usize vmo_ofs, MmuFlags mmuf, ustl::Rc<VmObject> vmo, 
                            char const *name, ustl::Rc<VmMapping> *out) -> Status;

        /// Create a mapping of the specified size.
        auto create_mapping(usize size, usize vmo_ofs, MmuFlags mmuf, ustl::Rc<VmObject> vmo, 
                            char const *name, ustl::Rc<VmMapping> *out) -> Status;

        /// Create a mapping of the specified size.
        auto create_mapping(usize size, MmuFlags mmuf, char const *name, ustl::Rc<VmMapping> *out) 
            -> Status;

        /// Create a sub-VMA in page range [vma_off, vma_off + nr_pages).
        auto create_subvma(usize offset, usize size, VmaFlags vmaf, char const *name, ustl::Rc<VmArea> *out) 
            -> Status;

        /// Create a sub-VMA of the specified size.
        auto create_subvma(usize nr_pages, VmaFlags, char const *name, ustl::Rc<VmArea> *) -> Status;

        auto unmap(usize offset, usize size) -> Status;

        /// Update rights of the subset of the area. 
        auto protect(usize offset, usize size, MmuFlags flags) -> Status;

        /// Update rights of the subset of the area. 
        auto reserve(usize offset, usize size, MmuFlags flags, char const *name) -> Status;

        auto contains(VirtAddr addr) const -> bool;
    protected:
        VmArea(VirtAddr, usize, VmaFlags, VmArea *, VmAspace *, char const *);

        virtual auto activate() -> void override; 
        virtual auto destroy() -> void override; 
    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmMapping;

        VmaSet subvmas_;
    };
} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP