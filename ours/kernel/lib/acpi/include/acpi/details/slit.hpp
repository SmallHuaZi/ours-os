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

#ifndef ACPI_DETAILS_SLIT_HPP
#define ACPI_DETAILS_SLIT_HPP 1

#include <acpi/details/header.hpp>

namespace acpi {
    struct PACKED AcpiSlit {
        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize {
            return header.size();
        }

        AcpiTableHeader header;
	    u64 locality_count;

        CXX11_CONSTEXPR
        static AcpiSignature const SIGNATURE{"SLIT"};
    };

} // namespace acpi

#endif // #ifndef ACPI_DETAILS_SLIT_HPP