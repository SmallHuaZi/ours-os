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

#include <ours/mem/types.hpp>
#include <ours/mem/vm_fault.hpp>

#include <ustl/rc.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmaFlags: u64 {
        // Status.
        Inactive = 0,
        Active   = 1,

        // Features.
        Anonymous = 0x40000,
        Mergeable = 0x80000,
        Sharable = 0x111,

        // Categories
        Normal   = 0x10000,
        Mapped   = 0x20000,
    };
    USTL_ENABLE_ENUM_BITMASK(VmaFlags);

    /// Specific handler for various regions, to avoid the use of virtual function, 
    /// which would lead many meaningless derived classes.
    struct VmAreaHandler
    {
        mutable ustl::function::Fn<auto () -> void>  open;
        mutable ustl::function::Fn<auto () -> void>  close;

        //! @page_fault_: Addresses a page fault happened in `VmRootArea' which is holding this.
        typedef ustl::function::Fn<auto (VmFault *) -> void>   PageFaultFn;
        mutable PageFaultFn  fault;
    };

    /// `VmArea` is a representation of a contiguous range of virtual memory space.
    class VmArea
        : public ustl::RefCounter<VmArea>
    {
        typedef VmArea    Self;
        typedef ustl::RefCounter<VmArea>    Base;
    public:
        // Being protected is to avoid the incorrect use.
        VmArea(ustl::Rc<VmAspace>, VirtAddr, usize, MmuFlags, VmaFlags, char const *);

        auto activate() -> void;

        auto destroy() -> void;

        /// Let start = range_.start() + offset, end = start + len.
        /// Map [start, end) to physical memory.
        auto map_range(usize offset, usize len, MmuFlags flags) -> void;

        auto unmap(usize base, usize size) -> void;

        auto protect(usize base, usize size, MmuFlags flags) -> void;

        auto contains(VirtAddr addr) const -> bool;

        auto address_range() const -> gktl::Range<VirtAddr>;

        auto is_anony() const -> bool
        {  return static_cast<bool>(this->flags_ & VmaFlags::Anonymous);  }

        auto fault(VmFault *vmf) const -> void
        {  return handler_->fault(vmf);  }

    protected:
        auto split() -> ustl::Rc<VmArea>;

    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmAreaHandler;
        friend class VmRootArea;

        GKTL_CANARY(VmArea, canary_);
        char const *name_;
        VirtAddr  base_;
        usize     size_;
        VmaFlags  flags_;
        MmuFlags  rights_;
        VmAreaHandler       *handler_;
        ustl::Rc<VmAspace>   aspace_;
        ustl::Rc<VmObject>   vmo_;
        usize object_pgoff_;
        ustl::collections::intrusive::SetMemberHook<>   set_hook_;
        ustl::collections::intrusive::ListMemberHook<>  list_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, set_hook_, ManagedSetOptions);
        USTL_DECLARE_HOOK_OPTION(Self, list_hook_, ManagedListOptions);
    };
    USTL_DECLARE_MULTISET(VmArea, VmaSet, VmArea::ManagedSetOptions);
    USTL_DECLARE_LIST(VmArea, VmaList, VmArea::ManagedListOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP