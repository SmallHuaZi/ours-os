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

#ifndef ARCH_TYPES_HPP
#define ARCH_TYPES_HPP 1

#include <ours/assert.hpp>
#include <ours/status.hpp>
#include <ours/macro_abi.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/arch_vm_aspace_constraints.hpp>

namespace arch {
    /// The follwoing types which was defined in subnamespaces of `ours`
    /// is immigrating into `arch` namespace grudually.
    using ours::u8;
    using ours::u16;
    using ours::u32;
    using ours::u64;
    using ours::usize;
    using ours::isize;
    using ours::usize;
    using ours::Status;
    using ours::PhysAddr;
    using ours::VirtAddr;
    using ours::mem::MapControl;
    using ours::mem::UnMapControl;
    using ours::mem::HarvestControl;
    using ours::mem::VmasFlags;
    using ours::mem::MmuFlags;
    using ours::mem::Altmap;
    using ours::HIrqNum;
    using ours::VIrqNum;
    typedef ours::usize   Pte;

} // namespace arch

#endif // #ifndef ARCH_TYPES_HPP