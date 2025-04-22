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
    /// When a or a group of frame was used in VMM, it shows the following layout.
    struct VmPage: public PageFrameBase {
        typedef VmPage  Self;

        VmObject *vmo; // For reverse mapping.
        ustl::sync::AtomicU32 vmo_index;    // Index in VMO's page list
        ustl::sync::AtomicU16 num_mappings;
        ustl::sync::AtomicU16 num_users;
    };
    static_assert(sizeof(VmPage) <= kFrameDescSize, "");

    template <>
    struct RoleViewDispatcher<PfRole::Vmm> {
        typedef VmPage    Type;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_HPP