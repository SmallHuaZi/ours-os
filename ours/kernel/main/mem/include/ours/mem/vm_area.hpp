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

#include <ktl/result.hpp>
#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    /// `VmArea` is a representation of a contiguous range of virtual memory space.
    class VmArea: public VmAreaOrMapping {
        typedef VmArea          Self;
        typedef VmAreaOrMapping Base;
    public:
        FORCE_INLINE
        static auto create(VirtAddr va, usize size, VmaFlags vmaf, VmArea *parent, VmAspace *aspace, 
                           char const *name, ustl::Rc<Self> *out) -> Status {
            DEBUG_ASSERT(parent);
            return create_common(va, size, vmaf, parent, aspace, name, out);
        }

        FORCE_INLINE
        static auto create_root(VirtAddr va, usize size, VmaFlags vmaf, VmAspace *aspace, char const *name, 
                                ustl::Rc<Self> *out) -> Status {
            return create_common(va, size, vmaf, nullptr, aspace, name, out);
        }

        /// Create a mapping unit in page range [vma_off, vma_off + size).
        auto create_mapping(usize vma_ofs, usize size, usize vmo_ofs, MmuFlags mmuf, ustl::Rc<VmObject> vmo, 
                            char const *name, VmMapOption option, ustl::Rc<VmMapping> *out) -> Status;

        /// Create a sub-VMA in page range [vma_off, vma_off + nr_pages).
        auto create_subvma(usize offset, usize size, VmaFlags vmaf, char const *name, VmMapOption option, 
                          ustl::Rc<VmArea> *out) -> Status;

        /// Following map* interfaces are the a group of coarse-grained method to provide a convenient way for 
        /// mapping a segment of VMA.
        ///
        /// When offer option |VmMapOption::Fixed| the range [base, base + size) would be forcely 
        /// used, namely those existing mappings which overlaps with it will be replaced with expected one.
        ///
        /// When offer option |VmMapOption::Commit|, this mapping request will be committed intermediately, 
        /// rather than wait a page fault to trigger real mapping action. 
        ///
        /// On success them return a object of VmMapping.

        /// Map |size| bytes space from the given physical address |phys_base| to virtual address |virt_base|.
        auto map_at(PhysAddr phys_base, VirtAddr ai_out *virt_base, usize size, MmuFlags, VmMapOption, char const *name)
            -> ktl::Result<ustl::Rc<VmMapping>>;

        /// Map |size| bytes space to virtual address |virt_base|.
        auto map(VirtAddr ai_out *base, usize size, MmuFlags, VmMapOption, char const * name)
            -> ktl::Result<ustl::Rc<VmMapping>>;

        auto unmap(usize offset, usize size) -> Status;

        /// Update rights of the subset of the area. 
        auto protect(usize offset, usize size, MmuFlags flags) -> Status;

        /// Update rights of the subset of the area. 
        auto reserve(usize offset, usize size, MmuFlags flags, char const *name) -> Status;

        auto contains(VirtAddr addr) const -> bool;

        auto dump() const -> void;
    protected:
        static auto create_common(VirtAddr, usize, VmaFlags, VmArea *, VmAspace *, char const *, ustl::Rc<Self> *) 
            -> Status;

        VmArea(VirtAddr, usize, VmaFlags, VmArea *, VmAspace *, char const *);

        auto alloc_spot(usize size, AlignVal align, VirtAddr lower_limit, VirtAddr upper_limit) -> ktl::Result<VirtAddr>;

        struct CreateVmAomArgs;
        /// Create a sub-VMA or sub-Mapping and do not check the given range in packet
        auto create_subaom_internal(CreateVmAomArgs &packet, ustl::Rc<VmAreaOrMapping> *out) -> Status;

        auto map_with_vmo(VirtAddr *, usize, MmuFlags, ustl::Rc<VmObject>, VmMapOption, char const *)
            -> ktl::Result<ustl::Rc<VmMapping>>;
        
        virtual auto destroy() -> Status override;
    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmMapping;
        friend class VmAreaOrMapping;

        usize num_mappings_;
        VmaSet subvmas_;
    };
} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP