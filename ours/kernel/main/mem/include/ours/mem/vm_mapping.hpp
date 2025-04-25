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

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    class ProtectedRegionSet {
    public:
        
    private:
        struct ProtectedNode: public ustl::collections::intrusive::SetBaseHook<> {
            VirtAddr size;
            MmuFlags mmuf;
        };
        ustl::collections::intrusive::Set<ProtectedNode> regions_;
    };

    /// VmMapping is the representation of a or a group of area which has been mapped in 
    /// virtual memory address space.
    /// 
    /// It was usually created by VmArea out of some mapping request.
    class VmMapping: public ustl::RefCounter<VmMapping> {
        typedef VmMapping   Self;
    public:
        static auto create(VirtAddr, usize, VmArea *, PgOff, ustl::Rc<VmObject>, MmuFlags, char const *, ustl::Rc<Self> *) 
            -> Status;
        
        auto map(PgOff, usize nr_pages, bool commit, MapControl control) -> Status;

        auto protect(PgOff, usize nr_pages, usize size, MmuFlags mmuf) -> Status;

        auto unmap(PgOff, usize nr_pages, UnmapControl control) -> Status;

        auto fault(VmFault *vmf) -> void;

        FORCE_INLINE
        auto aspace() -> ustl::Rc<VmAspace> {
            return aspace_;
        }

        FORCE_INLINE
        auto base() const -> VirtAddr {
            return base_;
        }

        FORCE_INLINE
        auto size() const -> VirtAddr {
            return size_;
        }

        VmMapping(VirtAddr, usize, VmArea *, PgOff, ustl::Rc<VmObject>, MmuFlags, char const *);
        virtual ~VmMapping() = default;
    private:
        friend class VmArea;
        friend class VmObject;
        
        auto map_paged(PgOff pgoff, usize nr_pages, bool commit, MapControl control, VmObjectPaged *vmo) -> Status;

        auto activate() -> void;

        auto check_sburange(PgOff pgoff, usize nr_pages) const -> bool;

        GKTL_CANARY(VmMapping, canary_);
        MmuFlags mmuf_;
        VirtAddr base_;
        VirtAddr size_;
        ustl::Rc<VmArea> vma_;
        ustl::Rc<VmObject> vmo_;
        ustl::Rc<VmAspace> aspace_;
        PgOff vmo_pgoff_;
        ProtectedRegionSet protected_regions_;
        ustl::collections::intrusive::ListMemberHook<> list_hook_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, list_hook_, VmoListHookOptions);
    };
    USTL_DECLARE_LIST(VmMapping, VmMappingList, VmMapping::VmoListHookOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_MAPPING_HPP