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
#ifndef OURS_PLATFORM_ACPI_HPP
#define OURS_PLATFORM_ACPI_HPP 1

#include <acpi/parser.hpp>

namespace ours {
    FORCE_INLINE CXX11_CONSTEXPR
    auto get_acpi_parser() -> acpi::IAcpiParser * {
        extern acpi::IAcpiParser *g_acpi_parser;
        return g_acpi_parser;
    }

    auto acpi_init_numa() -> Status;

} // namespace ours

#endif // #ifndef OURS_PLATFORM_ACPI_HPP