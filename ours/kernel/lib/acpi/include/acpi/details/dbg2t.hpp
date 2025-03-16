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
#ifndef ACPI_DETAILS_DBG2T_HPP
#define ACPI_DETAILS_DBG2T_HPP 1

#include <acpi/details/header.hpp>

namespace acpi {
    // DBG2 table
    struct PACKED AcpiDbg2Table {
        AcpiTableHeader header;
        u32 offset;
        u32 num_entries;

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize { 
            return header.length; 
        }
        static constexpr auto SIGNATURE = AcpiSignature("DBG2");
    };
    static_assert(sizeof(AcpiDbg2Table) == 44);

    enum class AcpiDbg2Type: u16 {
        SerialPort = 0x8000,
        T1394Port = 0x8001,
        UsbPort = 0x8002,
        NetPort = 0x8003,
    };

    enum class AcpiDbg2SubType: u16 {
        T16550Compatible = 0x0000,
        T16550Subset = 0x0001,
        T1394Standard = 0x0000,
        UsbXhci = 0x0000,
        UsbEhci = 0x0001,
    };

    struct PACKED AcpiDbg2Device {
        u8 revision;
        u16 length;
        u8 register_count;
        u16 namepath_length;
        u16 namepath_offset;
        u16 oem_data_length;
        u16 oem_data_offset;
        u16 port_type;
        u16 port_subtype;
        u16 reserved;
        u16 base_address_offset;
        u16 address_size_offset;

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize { 
            return length; 
        }
    };
    static_assert(sizeof(AcpiDbg2Device) == 22);

} // namespace acpi

#endif // #ifndef ACPI_DETAILS_DBG2T_HPP