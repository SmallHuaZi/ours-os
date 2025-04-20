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
#ifndef OURS_MEM_VM_ROOT_AREA_HPP
#define OURS_MEM_VM_ROOT_AREA_HPP 1

#include <ours/status.hpp>
#include <ours/mem/vm_area.hpp>

namespace ours::mem {
    /// This class is used to manage the sub-area in a virtual address space.
    class VmRootArea {
        typedef VmRootArea    Self;
    public:
        static auto create(ustl::Rc<VmAspace>, VmaFlags) -> ustl::Rc<VmRootArea>;

        /// Create a mapping area from `range`.
        auto create_subvma(VirtAddr base, usize size, usize align, MmuFlags flags, ustl::Rc<VmObject> * = 0) 
            -> ustl::Rc<VmArea>;

        auto destory_subvma(VirtAddr base, usize size) -> Status;

        auto reserve_subvma(char const *name, VirtAddr base, usize size, MmuFlags flags) -> Status;

        auto protect_subvma(VirtAddr base, usize size, MmuFlags flags) -> Status;

        auto find_subvma(VirtAddr va) -> ustl::Rc<VmArea>;
    private:
        auto create_subvma_inner(VirtAddr base, usize size, usize align, MmuFlags flags) -> ustl::Rc<VmArea>;
    
    private:
        VmaSet   subvma_set_;
        ustl::Rc<VmAspace>   aspace_;
        gktl::Range<VirtAddr> range_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_ROOT_AREA_HPP