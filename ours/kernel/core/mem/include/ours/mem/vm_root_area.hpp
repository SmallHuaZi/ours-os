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

#include <ours/mem/vm_area.hpp>

namespace ours::mem {
    /// This class is used to manage the sub-area in a virtual address space.
    class VmRootArea
    {
        typedef VmRootArea    Self;
    public:
        static auto create(ustl::Rc<VmAspace>, VmaFlags) -> ustl::Rc<VmRootArea>;

        auto create_subvma(gktl::Range<VirtAddr> range, usize align, VmaFlags flags) -> ustl::Rc<VmArea>;

        /// Create a mapping area from `range`.
        auto create_subvma(gktl::Range<VirtAddr> range, usize align, VmaFlags, ustl::Rc<VmObject> *) -> ustl::Rc<VmArea>;

        auto destory_subvma(gktl::Range<VirtAddr> range) -> Status;

        auto protect_subvma(gktl::Range<VirtAddr> range, VmAreaRight flags) -> Status;

        auto find_subvma(VirtAddr va) -> ustl::Rc<VmArea>;
    
    private:
        auto create_subvma_inner(gktl::Range<VirtAddr> range, usize align, VmaFlags flags) -> ustl::Rc<VmArea>;
    
    private:
        VmAreaSet   subvma_set_;
        gktl::Range<VirtAddr> range_;
        ustl::Rc<VmAspace>   aspace_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_ROOT_AREA_HPP