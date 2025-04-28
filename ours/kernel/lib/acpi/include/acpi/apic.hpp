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
#ifndef ACPI_APIC_HPP
#define ACPI_APIC_HPP 1

#include <acpi/parser.hpp>
#include <acpi/details/madt.hpp>
#include <ours/status.hpp>

#include <ustl/function/fn.hpp>

namespace acpi {
    // The following functions is particular to x86 architecture.
    // BUG(SmallHuaZi) To `ustl::function::Fn`, please see docs/workaround/ustl.md 
    typedef ustl::function::Fn<auto(AcpiMadtIoApicEntry const &) -> void> IoApicCommitFn;
    auto enumerate_io_apics(AcpiMadt const &, IoApicCommitFn const &commit) -> ours::Status;

    FORCE_INLINE
    static auto enumerate_io_apics(IAcpiParser const &parser, IoApicCommitFn const &commit) -> ours::Status {
        auto const madt = get_table_by_signature(parser, AcpiMadt::kSignature);
        if (!madt) {
            return Status::NotFound;
        }

        return enumerate_io_apics(*reinterpret_cast<AcpiMadt const *>(madt), commit);
    }

    typedef ustl::function::Fn<auto(AcpiMadtLocalApicEntry const &) -> void> LocalApicCommitFn;
    auto enumerate_local_apics(AcpiMadt const &, LocalApicCommitFn const &commit) -> ours::Status;

    FORCE_INLINE
    static auto enumerate_local_apics(IAcpiParser const &parser, LocalApicCommitFn const &commit) -> ours::Status {
        auto const madt = get_table_by_signature(parser, AcpiMadt::kSignature);
        if (!madt) {
            return Status::NotFound;
        }

        return enumerate_local_apics(*reinterpret_cast<AcpiMadt const *>(madt), commit);
    }

    typedef ustl::function::Fn<auto(AcpiMadtInterruptOverrideEntry const &) -> void> IntrOverridesCommitFn;
    auto enumerate_io_apic_isa_overrides(AcpiMadt const &, IntrOverridesCommitFn const &commit) -> ours::Status;

    FORCE_INLINE
    static auto enumerate_io_apic_isa_overrides(IAcpiParser const &parser, IntrOverridesCommitFn const &commit) 
        -> ours::Status {
        auto const madt = get_table_by_signature(parser, AcpiMadt::kSignature);
        if (!madt) {
            return Status::NotFound;
        }

        return enumerate_io_apic_isa_overrides(*reinterpret_cast<AcpiMadt const *>(madt), commit);
    }

} // namespace acpi

#endif // #ifndef ACPI_APIC_HPP