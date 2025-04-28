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
#ifndef ACPI_MADT_HPP
#define ACPI_MADT_HPP 1

#include <acpi/details/header.hpp>

namespace acpi {
    // Define the MADT entry types
    enum AcpiMadtType : u8 {
        AcpiMadtTypeLocalApic,
        AcpiMadtTypeIoApic,
        AcpiMadtTypeInterruptOverride,
        AcpiMadtTypeNmi = 3,
        AcpiMadtTypeLocalApicNmi = 4,
        AcpiMadtTypeIoSapic = 5,
        AcpiMadtTypeLocalSapic = 6,
        AcpiMadtTypePlatformInterruptSource = 7,
        AcpiMadtTypeLocalX2Apic = 8,
        AcpiMadtTypeIoX2Apic = 9,
        AcpiMadtTypeLocalX2ApicNmi = 10,
        AcpiMadtTypeGicCpuInterface = 11,
        AcpiMadtTypeGicDistributor = 12,
        AcpiMadtTypeGicMsiFrame = 13,
        AcpiMadtTypeGicRedistributor = 14,
        AcpiMadtTypeGicExtendedRegister = 15
    };

    /// Entry type `InterruptOverride`, `Nmi` and `LocalApicNmi` have a flags field, which is 
    /// useful for settings up the I/O APIC redirection entry or local vector table entry 
    /// respectively. If (flags & 2) then the interrupt is active when low, and if (flags & 8) 
    /// then interrupt is level-triggered.
    enum class ApicIntrType: u8 {
        EdgeHigh = 0b0000,
        EdgeLow = 0b0010,
        LevelHigh = 0b1000,
        LevelLow = 0b1010,
    };

    enum ApicMadtFlag: u8 {
        AcpiMadtFlagDisabled,
        AcpiMadtFlagEnabled,
    };

    // MADT Entry Type 0: Processor Local APIC
    struct PACKED AcpiMadtLocalApicEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id;
        u8 apic_id;

        // Flags (bit 0 = Processor Enabled) (bit 1 = Online Capable)
        u32 flags; 
    };

    // MADT Entry Type 1: I/O APIC
    struct PACKED AcpiMadtIoApicEntry {
        AcpiEntryHeader header;
        u8 io_apic_id;        // I/O APIC ID
        u8 reserved;          // Reserved
        u32 io_apic_address;  // I/O APIC address
        u32 global_irq_base;  // Global interrupt base
    };

    #define ACPI_MADT_FLAG_POLARITY_CONFORMS 0b00
    #define ACPI_MADT_FLAG_POLARITY_HIGH 0b01
    #define ACPI_MADT_FLAG_POLARITY_LOW 0b11
    #define ACPI_MADT_FLAG_POLARITY_MASK 0b11

    #define ACPI_MADT_FLAG_TRIGGER_CONFORMS 0b0000
    #define ACPI_MADT_FLAG_TRIGGER_EDGE 0b0100
    #define ACPI_MADT_FLAG_TRIGGER_LEVEL 0b1100
    #define ACPI_MADT_FLAG_TRIGGER_MASK 0b1100

    // MADT Entry Type 2: Interrupt Override
    struct PACKED AcpiMadtInterruptOverrideEntry {
        AcpiEntryHeader header;
        u8 bus;               // Bus number
        u8 source;            // Interrupt source number
        u32 global_irq;       // Global interrupt number
        u8 flags;             // Flags

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_intr_type() -> ApicIntrType {
            return ApicIntrType(flags & 0b1010);
        }
    };

    // MADT Entry Type 3: NMI Interrupt
    struct PACKED MadtNmiEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id; // ACPI processor ID
        u16 flags;            // Flags
        u8 global_irq;        // Global interrupt number

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_intr_type() -> ApicIntrType {
            return ApicIntrType(flags & 0b1010);
        }
    };

    // MADT Entry Type 4: Local APIC NMI
    struct PACKED MadtLocalApicNmiEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id; // ACPI processor ID
        u16 flags;            // Flags
        u8 local_apic_lint;   // Local APIC LINT pin

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_intr_type() -> ApicIntrType {
            return ApicIntrType(flags & 0b1010);
        }
    };

    // MADT Entry Type 5: I/O SAPIC
    struct PACKED MadtIoSapicEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id; // ACPI processor ID
        u8 io_sapic_id;       // I/O SAPIC ID
        u32 flags;            // Flags
        u32 io_sapic_address; // I/O SAPIC address
    };

    // MADT Entry Type 6: Local SAPIC
    struct PACKED MadtLocalSapicEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id; // ACPI processor ID
        u8 local_sapic_id;    // Local SAPIC ID
        u8 local_sapic_eid;   // Local SAPIC EID
        u32 flags;            // Flags
        u32 reserved[2];      // Reserved
    };

    // MADT Entry Type 7: Platform Interrupt Source
    struct PACKED MadtPlatformInterruptSourceEntry {
        AcpiEntryHeader header;
        u8 type;              // Interrupt source type
        u8 reserved;          // Reserved
        u16 flags;            // Flags
        u32 global_irq;       // Global interrupt number
    };

    // MADT Entry Type 8: Processor Local x2APIC
    struct PACKED MadtLocalX2ApicEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 apic_id;           // x2APIC ID
        u32 flags;            // Flags
        u32 acpi_processor_id;// ACPI processor ID
    };

    // MADT Entry Type 9: I/O x2APIC
    struct PACKED MadtIoX2ApicEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 io_apic_id;        // I/O x2APIC ID
        u32 flags;            // Flags
        u32 io_apic_address;  // I/O x2APIC address
        u32 global_irq_base;  // Global interrupt base
    };

    // MADT Entry Type 10: Local x2APIC NMI
    struct PACKED MadtLocalX2ApicNmiEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 apic_id;           // x2APIC ID
        u16 flags;            // Flags
        u8 local_apic_lint;   // Local x2APIC LINT pin
    };

    // MADT Entry Type 11: GIC CPU Interface
    struct PACKED MadtGicCpuInterfaceEntry {
        AcpiEntryHeader header;
        u8 acpi_processor_id; // ACPI processor ID
        u8 flags;             // Flags
        u16 reserved;         // Reserved
        u32 gic_cpu_interface_number; // GIC CPU interface number
    };

    // MADT Entry Type 12: GIC Distributor
    struct PACKED MadtGicDistributorEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 gic_distributor_id;// GIC Distributor ID
        u32 flags;            // Flags
        u32 gic_distributor_base_address; // GIC Distributor base address
    };

    // MADT Entry Type 13: GIC MSI Frame
    struct PACKED MadtGicMsiFrameEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 gic_msi_frame_id;  // GIC MSI Frame ID
        u32 flags;            // Flags
        u32 gic_msi_frame_base_address; // GIC MSI Frame base address
        u32 reserved2;        // Reserved
    };

    // MADT Entry Type 14: GIC Redistributor
    struct PACKED MadtGicRedistributorEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 gic_redistributor_id; // GIC Redistributor ID
        u32 flags;            // Flags
        u32 gic_redistributor_base_address; // GIC Redistributor base address
        u32 reserved2;        // Reserved
    };

    // MADT Entry Type 15: GIC Extended Register
    struct PACKED MadtGicExtendedRegisterEntry {
        AcpiEntryHeader header;
        u8 reserved[3];       // Reserved
        u8 gic_extended_register_id; // GIC Extended Register ID
        u32 flags;            // Flags
        u32 gic_extended_register_base_address; // GIC Extended Register base address
        u32 reserved2;        // Reserved
    };

    /// Multiple APIC Description Table
    struct PACKED AcpiMadt {
        CXX11_CONSTEXPR
        auto size() const -> usize {
            return header.length;
        }

        AcpiTableHeader header;
        u32 lapic_addr;
        u32 flags;

        CXX11_CONSTEXPR
        static auto const kSignature = AcpiSignature("APIC");
    };

} // namespace acpi

#endif // #ifndef ACPI_MADT_HPP