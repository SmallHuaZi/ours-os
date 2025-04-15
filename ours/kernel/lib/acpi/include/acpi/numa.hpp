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
#include <acpi/details/slit.hpp>
#include <ustl/function/fn.hpp>

namespace acpi {
    struct NumaRegion {
        u64 base;
        u64 size;
    };

    using ours::Status; 

    FORCE_INLINE
    static auto count_numa_domains(IAcpiParser const &parser) -> usize {
        auto const header = get_table_by_signature(parser, AcpiSlit::kSignature);
        if (!header) {
            return 0;
        }

        auto slit = reinterpret_cast<AcpiSlit const *>(header);
        return slit->locality_count;
    }

    typedef ustl::function::Fn<void(u32 numa_domain, NumaRegion const &)> NumaRegionCommitFn;
    auto enumerate_numa_region(AcpiSrat const &, NumaRegionCommitFn const &) -> Status;

    FORCE_INLINE
    static auto enumerate_numa_region(IAcpiParser const &parser, NumaRegionCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSrat::kSignature);
        if (!srat) {
            return Status::NotFound;
        }

        return enumerate_numa_region(*reinterpret_cast<AcpiSrat const *>(srat), commit);
    }

    typedef ustl::function::Fn<void(u32 cpunum, u32 numa_domain)> CpuNumaPairsCommitFn;
    auto enumerate_cpu_numa_pairs(AcpiSrat const &, CpuNumaPairsCommitFn const &) -> Status;

    FORCE_INLINE
    static auto enumerate_cpu_numa_pairs(IAcpiParser const &parser, CpuNumaPairsCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSrat::kSignature);
        if (!srat) {
            return Status::NotFound;
        }
        return enumerate_cpu_numa_pairs(*reinterpret_cast<AcpiSrat const *>(srat), commit);
    }

    typedef ustl::function::Fn<void(u32 from, u32 to, u8 distance)> NumaDistanceCommitFn;
    auto enumerate_numa_domain_distance(AcpiSlit const &slit, NumaDistanceCommitFn const &) -> Status;

    FORCE_INLINE
    static auto enumerate_numa_domain_distance(IAcpiParser const &parser, NumaDistanceCommitFn const &commit) -> Status {
        auto const slit= get_table_by_signature(parser, AcpiSlit::kSignature);
        if (!slit)  {
            return Status::NotFound;
        }
        return enumerate_numa_domain_distance(*reinterpret_cast<AcpiSlit const *>(slit), commit);
    }

} // namespace acpi

#endif // #ifndef ACPI_NUMA_HPP