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
#ifndef OURS_MEM_VM_MAPPING_HPP
#define OURS_MEM_VM_MAPPING_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/fault.hpp>
#include <ours/mem/vm_area_base.hpp>

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    /// Specific handler for various regions, to avoid the use of virtual function, 
    /// which would lead many meaningless derived classes.
    struct VmMappingHandler {
        mutable ustl::function::Fn<auto () -> void>  open;
        mutable ustl::function::Fn<auto () -> void>  close;

        //! @page_fault_: Addresses a page fault happened in `VmRootArea' which is holding this.
        typedef ustl::function::Fn<auto (VmFault *) -> void>   PageFaultFn;
        mutable PageFaultFn  fault;
    };

    class VmMapping: public VmAreaBase {
        typedef VmMapping   Self;
        typedef VmAreaBase  Base;
    public:
        static auto create(VirtAddr, usize, VmArea *, VmaFlags, PgOff, ustl::Rc<VmObject>, MmuFlags, char const *name) 
            -> ustl::Result<ustl::Rc<Self>, Status>;
        
        auto map_range(PgOff pgoff, usize size, bool commit, MapControl control) -> Status;

        auto unmap_range(PgOff pgoff, usize size, UnMapControl control) -> Status;

        FORCE_INLINE
        auto fault(VmFault *vmf) -> void {
            DEBUG_ASSERT(handler_ && handler_->fault);
            handler_->fault(vmf);
        }

        VmMapping(VirtAddr base, usize, VmArea *, VmaFlags, PgOff, ustl::Rc<VmObject>, MmuFlags, char const *name);
    private:
        friend class VmArea;
        friend class VmObject;

        GKTL_CANARY(VmMapping, canary_);
        MmuFlags mmuf_;
        VmMappingHandler *handler_;
        ustl::Rc<VmArea> vma_;
        ustl::Rc<VmObject> vmo_;
        PgOff vmo_pgoff_;

        ustl::collections::intrusive::SetMemberHook<> vma_set_hook_;
        ustl::collections::intrusive::ListMemberHook<> vmo_list_hook_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, vma_set_hook_, VmaSetHookOptions);
        USTL_DECLARE_HOOK_OPTION(Self, vmo_list_hook_, VmoListHookOptions);
    };
    USTL_DECLARE_LIST(VmMapping, VmMappingList, VmMapping::VmoListHookOptions);
    USTL_DECLARE_MULTISET(VmMapping, VmMappingSet, VmMapping::VmaSetHookOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_MAPPING_HPP