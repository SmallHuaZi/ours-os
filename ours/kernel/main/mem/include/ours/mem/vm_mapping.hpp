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
    /// Helper class to trace the mapping status in region. Its internal implementation
    /// is based on multiple adjacent intervals whose left bound is the end of the previous one.
    class MappingRegionSet {
        typedef MappingRegionSet    Self;
      public:
        class Enumerator;

        auto init(VirtAddr end, MmuFlags) -> Status;

        /// Update MMU flags for the range [base, base + size)
        ///
        /// Return a enumerator that can assist us to for each all regions updated.
        auto update(VirtAddr base, usize size, MmuFlags, VirtAddr lower_bound, VirtAddr upper_bound) 
            -> Status;
        
        /// Make an enumerator which can enumerates all regions in range [base, base + size)
        auto make_enumerator(VirtAddr base, usize size)
            -> Enumerator;

        struct Region: public ustl::collections::intrusive::SetBaseHook<> {
            typedef Region  Self;
            typedef ustl::collections::intrusive::SetBaseHook<> Base;

            Region() = default;
            Region(VirtAddr end, MmuFlags mmuf)
                : Base(), end(end), mmuf(mmuf)
            {};

            VirtAddr end;
            MmuFlags mmuf;
        };

        FORCE_INLINE CXX11_CONSTEXPR
        friend auto operator<(Region const &x, Region const &y) -> bool {
            return x.end < y.end;
        }
      private:
        template <typename... Args>
        static auto alloc_region(Args &&...) -> Region *;

        static auto free_region(Region *) -> void;

        typedef ustl::collections::intrusive::MultiSet<Region>  RegionList;
        RegionList regions_;
    };

    class MappingRegionSet::Enumerator {
        typedef Enumerator  Self;
      public:
        Enumerator(VirtAddr base, RegionList::IterMut first, RegionList::IterMut last)
            : Self(base, first, last, 0, ustl::NumericLimits<VirtAddr>::max()) 
        {}

        Enumerator(VirtAddr base, RegionList::IterMut first, RegionList::IterMut last,
                   VirtAddr lower_bound, VirtAddr upper_bound)
            : base_(base), iter_(first), end_(last), 
              lower_bound_(base), upper_bound_(upper_bound)
        {}

        FORCE_INLINE
        auto shift(usize delta) -> void {
            base_ += delta;
        }

        struct NextResult {
            VirtAddr base;
            usize size;
            MmuFlags mmuf;
        };

        FORCE_INLINE
        auto next() -> ustl::Option<NextResult> {
            if (iter_ == end_ || base_ >= upper_bound_) {
                return ustl::none();
            }
            auto obase = ustl::algorithms::max(base_, lower_bound_);
            auto mmuf = iter_->mmuf;

            base_ = ustl::algorithms::min(iter_->end, upper_bound_);
            ++iter_;
            return ustl::some(NextResult{obase, base_ - obase, mmuf});
        }

        FORCE_INLINE
        friend auto operator!=(Self const &x, Self const &y) -> bool {
            return x.iter_ != x.iter_;
        }
      private:
        VirtAddr base_;
        VirtAddr lower_bound_;
        VirtAddr upper_bound_;
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

        VmMapping(VmArea *, VirtAddr, usize, VmaFlags, ustl::Rc<VmObject>, usize, char const *);
        virtual ~VmMapping() = default;
      private:
        friend class VmArea;
        friend class VmObject;

        auto map_paged(VmObjectPaged *vmo, VirtAddr base, usize size, bool commit, MapControl control) -> Status;

        auto map_physical(VmObjectPhysical *vmo, VirtAddr base, usize size, MapControl control) -> Status;

        virtual auto activate() -> Status override;
        virtual auto destroy() -> Status override;

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