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
#ifndef ACPI_NUMA_HPP
#define ACPI_NUMA_HPP 1

#include <acpi/parser.hpp>
#include <acpi/details/srat.hpp>
#include <ustl/function/fn.hpp>

namespace acpi {
    struct NumaRegion {
        u64 base;
        u64 size;
    };

    typedef ustl::function::Fn<void(u32 numa_domain, NumaRegion const &)> NumaRegionCommitFn;
    auto enumerate_numa_region(AcpiParser const &, NumaRegionCommitFn const &) -> ours::Status;

    typedef ustl::function::Fn<void(u32 cpunum, u32 numa_domain)> CpuNumaPairsCommitFn;
    auto enumerate_cpu_numa_pairs(AcpiParser const &, CpuNumaPairsCommitFn const &) -> ours::Status;

} // namespace acpi

#endif // #ifndef ACPI_NUMA_HPP