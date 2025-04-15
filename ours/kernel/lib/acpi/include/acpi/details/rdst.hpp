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
#ifndef ACPI_RDST_HPP
#define ACPI_RDST_HPP 1

#include <acpi/details/header.hpp>

namespace acpi {
    /// Root system description table pointer.
    struct PACKED AcpiRsdp {
        FORCE_INLINE CXX11_CONSTEXPR
        auto validate_signature() const -> bool {
            return sig1 == kSignature1 || sig2 == kSignature2;
        }

        auto validate() const -> bool;

        AcpiSignature sig1;
        AcpiSignature sig2;
        u8 checksum;
        u8 oemid[6];
        u8 revision;
        u32 rsdt_addr; // Deprected after the version 2.0

        CXX11_CONSTEXPR
        static AcpiSignature const kSignature1{"RSD "};

        CXX11_CONSTEXPR
        static AcpiSignature const kSignature2{"PTR "};
    };

    /// Extend system description table pointer.
    struct PACKED AcpiRsdpV2 {
        auto validate() const -> bool;

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize {
            return length;
        }

        AcpiRsdp v1;
        u32 length;
        u64 xsdt_addr;
        u8 ext_check_sum;
        u8 reserved[3];
    };

    struct PACKED AcpiRsdt {
        FORCE_INLINE CXX11_CONSTEXPR
        auto size() -> usize const {
            return header.length;
        }

        CXX11_CONSTEXPR 
        static AcpiSignature const kSignature{"RSDT"};

        AcpiTableHeader header;
        u32 addr32[0];
    };
    static_assert(sizeof(AcpiRsdt) == 36);

    struct PACKED AcpiXsdt {
        FORCE_INLINE CXX11_CONSTEXPR
        auto size() -> usize const {
            return header.length;
        }

        CXX11_CONSTEXPR 
        static AcpiSignature const kSignature{"XSDT"};

        AcpiTableHeader header;
        u64 addr64[0];
    };
    static_assert(sizeof(AcpiXsdt) == 36);

} // namespace acpi

#endif // #ifndef ACPI_RDST_HPP 