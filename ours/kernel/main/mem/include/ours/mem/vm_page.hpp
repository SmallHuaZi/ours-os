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
#ifndef OURS_MEM_VM_PAGE_HPP
#define OURS_MEM_VM_PAGE_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/vm_area.hpp>

#include <ustl/collections/intrusive/list.hpp>

namespace ours::mem {
    /// When a or group of frame was used in VMM, it shows the following layout.
    struct VmPage: public PmFrameBase {
        typedef VmPage  Self;

        VmaList vmas; // For reverse mapping.
        mutable ustl::sync::AtomicU16 num_mappings;
        mutable ustl::sync::AtomicU16 num_users;
        // Here are still a u32-sized space available.
        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedOptions);
    };
    static_assert(sizeof(VmPage) <= kFrameDescSize, "");
    USTL_DECLARE_LIST(VmPage, VmPageList, VmPage::ManagedOptions);

    template <>
    struct RoleViewDispatcher<PfRole::Vmm> {
        typedef VmPage    Type;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_HPP