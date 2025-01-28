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
#include <ours/mem/mmu_flags.hpp>

#include <ustl/rc.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    enum class VmaFlags: u64 {
        // Status.
        Inactive = 0,
        Active   = 1,

        // Features.
        Anony   = 0x40000,
        Merged  = 0x80000,

        // Categories
        Normal    = 0x10000,
        Mapping   = 0x20000,
    };
    USTL_ENABLE_ENUM_BITS(VmaFlags)

    enum class VmAreaRight: u64 {
        // Rights.
        Readable = 0x100,
        Writable = 0x200,
        Sharable = 0x400,
        Executable = 0x800,
    };
    USTL_ENABLE_ENUM_BITS(VmAreaRight)

    /// Specific handler for various regions, to avoid the use of  virtual function, 
    /// that would lead many meaningless derived  classes.
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
    public:
        // Being protected is to avoid the incorrect use.
        VmArea(ustl::Rc<VmAspace>, gktl::Range<VirtAddr>, VmAreaRight right);

        virtual auto activate() -> void;

        virtual auto destroy() -> void;

        /// Let start = range_.start() + offset, end = start + len.
        /// Map [start, end) to physical memory.
        auto map_range(usize offset, usize len, MmuFlags flags) -> void;

        auto unmap(usize base, usize size) -> void;

        auto protect(usize base, usize size, MmuFlags flags) -> void;

        auto contains(VirtAddr addr) const -> bool
        {  return this->range_.contains(addr);  }

        auto address_range() const -> gktl::Range<VirtAddr>
        {  return this->range_;  }

        auto is_anony() const -> bool
        {  return static_cast<bool>(this->flags_ & VmaFlags::Anony);  }

        auto fault(VmFault *vmf) const -> void
        {  return handler_->fault(vmf);  }

    protected:
        auto create_subvma_inner(gktl::Range<VirtAddr> range, usize align, VmaFlags flags) -> ustl::Rc<VmArea>;

        auto split() -> ustl::Rc<VmArea>;

    private:
        friend class VmObject;
        friend class VmAspace;
        friend class VmAreaHandler;
        friend class VmRootArea;

        gktl::Range<VirtAddr> range_;
        VmaFlags             flags_;
        VmAreaRight          rights_;
        VmAreaHandler       *handler_;
        ustl::Rc<VmAspace>   aspace_;
        ustl::Rc<VmObject>   object_;
        usize object_pgoff_;
        ustl::collections::intrusive::SetMemberHook<>     set_hook_;
        ustl::collections::intrusive::ListMemberHook<>    list_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, set_hook_, ManagedSetOptions);
        USTL_DECLARE_HOOK_OPTION(Self, list_hook_, ManagedListOptions);
    };
    USTL_DECLARE_MULTISET(VmArea, VmAreaSet, VmArea::ManagedSetOptions);
    USTL_DECLARE_LIST(VmArea, VmAreaList, VmArea::ManagedListOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_HPP