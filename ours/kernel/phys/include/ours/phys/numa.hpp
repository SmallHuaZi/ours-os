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
#ifndef OURS_PHYS_NUMA_HPP
#define OURS_PHYS_NUMA_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>

namespace ours::phys {
    /// It is a common routine to NUMA initialization in ACPI. If NUMA enabled,
    /// it should be invoked by `arch_init_numa()`.
    auto acpi_init_numa(PhysAddr rsdp) -> Status;

    /// The implementation provided by arch-code.
    auto arch_init_numa() -> void;

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_NUMA_HPP