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
#include <ours/mem/vm_area_or_mapping.hpp>

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/vec.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>
#include <ktl/allocator.hpp>

namespace ours::mem {
    /// Helper class to trace the mapping status in region.
    class MappingRegionSet {
        typedef MappingRegionSet    Self;

        struct Enumerator;
        struct Region {
            VirtAddr end;
            MmuFlags mmuf;
        };
        typedef ustl::collections::Vec<Region, ktl::Allocator<Region>>  RegionList;
      public:
        CXX11_CONSTEXPR
        static auto const kNumInitialRegions = 4;

        /// Update MMU flags for the range [base, base + size)
        auto update(VirtAddr base, usize size, MmuFlags, VirtAddr lower_bound, VirtAddr upper_bound) 
            -> Enumerator;

        MappingRegionSet(usize size, MmuFlags mmuf);
      private:
        RegionList regions_;
    };

    struct MappingRegionSet::Enumerator {
        typedef Enumerator  Self;
        struct NextResult {
            VirtAddr base;
            usize size;
            MmuFlags mmuf;
        };

        FORCE_INLINE
        auto shift(usize delta) -> void {
            base_ += delta;
        }

        FORCE_INLINE
        auto next() -> ustl::Option<NextResult> {
            if (iter_ != end_) {
                return ustl::none();
            }
            VirtAddr old_base = base_;
            MmuFlags mmuf = iter_->mmuf;

            base_ = iter_->end;
            ++iter_;
            return ustl::some(NextResult{base_, base_ - old_base, mmuf});
        }

        FORCE_INLINE
        friend auto operator!=(Self const &x, Self const &y) -> bool {
            return x.iter_ != x.iter_;
        }
      private:
        usize base_;

        RegionList::IterMut iter_;
        RegionList::IterMut end_;
    };

    /// VmMapping is the representation of a or a group of area which has been mapped in 
    /// virtual memory address space.
    /// 
    /// It was usually created by VmArea out of some mapping request.
    class VmMapping: public VmAreaOrMapping {
        typedef VmMapping       Self;
        typedef VmAreaOrMapping Base;
      public:
        static auto create(VmArea *, VirtAddr, usize, VmaFlags, ustl::Rc<VmObject>, usize, MmuFlags, 
                           char const *, ustl::Rc<Self> *) -> Status;
        
        auto map(VirtAddr offset, usize size, bool commit, MapControl) -> Status;

        auto protect(VirtAddr offset, usize size, MmuFlags) -> Status;

        auto unmap(VirtAddr offset, usize, UnmapControl) -> Status;

        auto fault(VmFault *vmf) -> void;

        VmMapping(VmArea *, VirtAddr, usize, VmaFlags, ustl::Rc<VmObject>, usize, MmuFlags, char const *);
        virtual ~VmMapping() = default;
      private:
        friend class VmArea;
        friend class VmObject;
        
        auto map_paged(VirtAddr base, usize size, bool commit, MapControl control, VmObjectPaged *vmo) -> Status;

        auto check_sburange(VirtAddr base, usize size) const -> bool;

        virtual auto activate() -> void override; 

        virtual auto destroy() -> void override; 

        ustl::Rc<VmObject> vmo_;
        usize vmo_off_;
        MappingRegionSet regions_;
        ustl::collections::intrusive::ListMemberHook<> list_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, list_hook_, VmoListHookOptions);
    };
    USTL_DECLARE_LIST(VmMapping, VmMappingList, VmMapping::VmoListHookOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_MAPPING_HPP