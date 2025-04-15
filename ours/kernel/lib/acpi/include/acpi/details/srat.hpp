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
#ifndef ACPI_SRAT_HPP
#define ACPI_SRAT_HPP 1

#include <acpi/details/header.hpp>

namespace acpi {
    enum AcpiSratType : u8 {
        AcpiSratTypeProcessorAffinity,
        AcpiSratTypeMemoryAffinity,
        AcpiSratTypeProcessorX2ApicAffinity,
        AcpiSratFlagEnabled = 1,
    };

    // SRAT table and descriptors.
    //
    // Reference: ACPI v6.3 Section 5.2.16.
    struct PACKED AcpiSrat {
        AcpiTableHeader header;
        u8 reserved[12];

        auto size() const -> usize {
            return header.length;
        }
        static constexpr auto kSignature = AcpiSignature("SRAT");
    };
    static_assert(sizeof(AcpiSrat) == 48);

    /// Type 0: processor local apic/sapic affinity structure
    ///
    /// Reference: ACPI v6.3 Section 5.2.16.1.
    struct PACKED AcpiSratProcessorAffinityEntry {
        AcpiEntryHeader header;
        u8 proximity_domain_low;
        u8 apic_id;
        u32 flags;
        u8 sapic_eid;
        u8 proximity_domain_high[3];
        u32 clock_domain;

        FORCE_INLINE CXX11_CONSTEXPR
        u32 proximity_domain() const {
            return u32(proximity_domain_low) |
                  (u32(proximity_domain_high[0]) << 8) |
                  (u32(proximity_domain_high[1]) << 16) |
                  (u32(proximity_domain_high[2]) << 24);
        }

        auto size() const -> usize {
            return header.length;
        }
    };
    static_assert(sizeof(AcpiSratProcessorAffinityEntry) == 16);

    /// Type 1: memory affinity structure
    ///
    /// Reference: ACPI v6.3 Section 5.2.16.2.
    struct PACKED AcpiSratMemoryAffinityEntry {
        AcpiEntryHeader header;
        u32 proximity_domain;
        u16 _reserved;
        u32 base_address_low;
        u32 base_address_high;
        u32 length_low;
        u32 length_high;
        u32 _reserved2;
        u32 flags;
        u32 _reserved3;
        u32 _reserved4;

        auto size() const -> usize {
            return header.length;
        }
    };
    static_assert(sizeof(AcpiSratMemoryAffinityEntry) == 40);

    /// Type 2: processor x2apic affinity structure
    ///
    /// Reference: ACPI v6.3 Section 5.2.16.3.
    struct PACKED AcpiSratProcessorX2ApicAffinityEntry {
        AcpiEntryHeader header;
        u16 _reserved;
        u32 proximity_domain;
        u32 x2apic_id;
        u32 flags;
        u32 clock_domain;
        u32 _reserved2;

        auto size() const -> usize {
            return header.length;
        }
    };
    static_assert(sizeof(AcpiSratProcessorX2ApicAffinityEntry) == 24);

} // namespace acpi

#endif // #ifndef ACPI_SRAT_HPP