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

#ifndef OURS_MEM_FAULT_HPP
#define OURS_MEM_FAULT_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/vm_fault.hpp>

namespace ours::mem {
    /// The common handling logic providing to arch-developer.
    ///
    /// @p addr(VirtAddr) is the virtual address that fault happened at.
    /// @p cause(VmfCause) explanates what lead to this fault.
    NO_MANGLE
    auto handle_vm_fault(VirtAddr addr, VmfCause cause) -> Status;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_FAULT_HPP