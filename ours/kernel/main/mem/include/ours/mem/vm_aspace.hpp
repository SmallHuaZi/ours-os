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
#ifndef OURS_MEM_VM_ASPACE_HPP
#define OURS_MEM_VM_ASPACE_HPP 1

#include <ours/config.hpp>
#if !__has_include(<ours/arch/vm_aspace.hpp>)
#   error "The header <ours/arch/vm_aspace.hpp> is required by [protocol.module.mem]"
#endif
#include <ours/arch/vm_aspace.hpp>

#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_root_area.hpp>
#include <ours/mem/vm_fault.hpp>
#include <ours/mem/arch_vm_aspace.hpp>

#include <ours/init.hpp>

#include <ustl/sync/mutex.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    static_assert(ArchVmAspaceConstraints<ArchVmAspace>());

    class VmAspace
        : public ustl::RefCounter<VmAspace>
    {
        typedef VmAspace       Self;
        typedef ustl::RefCounter<VmAspace>  Base;
    public:
        static auto clone(VmasFlags flags) -> ustl::Rc<VmAspace>;

        static auto create(VmasFlags flags, char const *) -> ustl::Rc<VmAspace>;

        static auto create(VirtAddr base, usize size, VmasFlags flags, char const *) -> ustl::Rc<VmAspace>;

        /// Perform the initialization of kernel address space.
        /// Just once called at boot-time.
        static auto init_kernel_aspace() -> void;

        /// Synchronize changes from kernel's address space into all user's.
        static auto sync_kernel_aspace() -> void;

        static auto switch_aspace(Self *, Self *) -> void;

        static auto kernel_aspace() -> ustl::Rc<VmAspace>
        {  return ustl::make_rc<VmAspace>(KERNEL_ASPACE_);  }

        auto is_user() const -> bool
        {  return static_cast<bool>(flags_ & VmasFlags::User);  }

        auto arch_aspace() -> ArchVmAspace & 
        {  return this->arch_;  }

        auto root_area() -> VmRootArea &
        {  return this->root_area_;  }

        auto init() -> Status;

        auto fault(VirtAddr addr, VmfCause flags) -> void;

        ~VmAspace();

    protected:
        auto map_object(ustl::Rc<VmObject> *, gktl::Range<VirtAddr>);
    
        VmAspace(VirtAddr, usize, VmasFlags, char const *);

    private:
        friend class VmArea;
        GKTL_CANARY(VmAspace, canary_);

        VirtAddr    base_;
        usize       size_;
        VmasFlags   flags_;

        /// Architecture specific context.
        ArchVmAspace  arch_;

        /// How many users hold this.
        ustl::sync::AtomicU32  users_;
        ustl::sync::AtomicU32  refcnt_;

        /// The only root region.
        VmRootArea          root_area_;
        ustl::Rc<VmArea>    fault_cache_;

        static VmAspace *KERNEL_ASPACE_;
        static ustl::sync::Mutex KERNEL_ASPACE_MUTEX_;

        /// The list which strungs all existing VmAspace.
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
        USTL_DECLARE_LIST(Self, AspaceList, ManagedOptions);
        static AspaceList ALL_ASPACE_LIST_;
        static ustl::sync::Mutex ALL_ASPACE_LIST_MUTEX_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_ASPACE_HPP