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

#include <ustl/collections/intrusive/set.hpp>

namespace ours::mem {
    /// `VmPage` is the implementation of the COW page
    class VmPage
    {
        typedef VmPage  Self;

    protected:
        PmFrame *frame_;
        ustl::collections::intrusive::SetMemberHook<> managed_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    };
    USTL_DECLARE_MULTISET(VmPage, VmPageSet, VmPage::ManagedOptions);

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_HPP