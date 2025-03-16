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
#ifndef ACPI_DEBUG_HPP
#define ACPI_DEBUG_HPP 1

#include <acpi/parser.hpp>
#include <acpi/details/dbg2t.hpp>
#include <ktl/result.hpp>

namespace acpi {
    struct DebugConfig {
        enum class Type: u8 {
            PortIo,
            Mmio,
        };

        Type type;
        // Physical address of the 16550 MMIO registers for Type::kMmio.
        // IO port base for Type::kPio.
        PhysAddr address;
        PhysAddr length;
    };

    auto get_debug_port(IAcpiParser const &parser) -> ktl::Result<DebugConfig>;

    auto parse_dbg2t(AcpiDbg2Table const &dbg2t) -> ktl::Result<DebugConfig>;

} // namespace acpi

#endif // #ifndef ACPI_DEBUG_HPP