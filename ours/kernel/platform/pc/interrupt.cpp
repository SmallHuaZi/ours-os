#include <ours/irq/mod.hpp>
#include <ours/mem/mod.hpp>
#include <ours/platform/acpi.hpp>
#include <ours/arch/apic.hpp>

#include <arch/x86/pic.hpp>
#include <arch/x86/apic/xapic.hpp>
#include <arch/x86/apic/ioapic.hpp>
#include <acpi/apic.hpp>

#include <gktl/init_hook.hpp>
#include <ktl/vec.hpp>

namespace ours {
    INIT_CODE
    static auto parse_io_apic_isa_overrides(acpi::AcpiMadtInterruptOverrideEntry const &entry) 
        -> arch::IoApicIsaOverride {
        arch::IoApicIsaOverride object;
        switch (entry.flags & ACPI_MADT_FLAG_TRIGGER_MASK) {
            case ACPI_MADT_FLAG_TRIGGER_CONFORMS:
            case ACPI_MADT_FLAG_TRIGGER_EDGE: 
                object.tri_mode = arch::ApicTriggerMode::Edge;
                break;
            case ACPI_MADT_FLAG_TRIGGER_LEVEL: 
                object.tri_mode = arch::ApicTriggerMode::Level;
                break;
            default:
                panic("Unknown IRQ trigger mode: {}.", entry.flags & ACPI_MADT_FLAG_TRIGGER_MASK);
        }

        switch (entry.flags & ACPI_MADT_FLAG_POLARITY_MASK) {
            case ACPI_MADT_FLAG_POLARITY_CONFORMS:
            case ACPI_MADT_FLAG_POLARITY_HIGH:
                object.polarity = arch::ApicInterruptPolarity::ActiveHigh;
                break;
            case ACPI_MADT_FLAG_POLARITY_LOW:
                object.polarity = arch::ApicInterruptPolarity::ActiveLow;
                break;
            default:
                panic("Unknown IRQ interrupt polarity: {}.", entry.flags & ACPI_MADT_FLAG_POLARITY_MASK);
        }

        object.global_irq = entry.global_irq;
        object.remapped = true;
        object.isa_irq = entry.source;
        return object;
    }

    INIT_CODE
    static auto platform_init_apic() -> void {
        // Before enable the APIC, we should shutdown the PIC.
        arch::Pic::disable();

        auto acpi_parser = get_acpi_parser();

        ktl::Vec<arch::IoApic> io_apics;
        acpi::enumerate_io_apics(*acpi_parser, [&] (acpi::AcpiMadtIoApicEntry const &entry) {
            io_apics.emplace_back().init(
                entry.io_apic_address, 
                mem::phys_to_pfn(PhysAddr(entry.io_apic_address)),
                entry.global_irq_base
            );
        });

        ktl::Vec<arch::IoApicIsaOverride> io_apic_isa_overrides;
        acpi::enumerate_io_apic_isa_overrides(*acpi_parser, [&] (acpi::AcpiMadtInterruptOverrideEntry const &entry) {
            io_apic_isa_overrides.emplace_back(parse_io_apic_isa_overrides(entry));
        });

        init_local_apic();
        init_io_apic(ustl::views::Span(io_apics), ustl::views::Span(io_apic_isa_overrides));

        using namespace arch;
        auto const bspid = current_apic_id(); 
        // Manually configing the ISA IRQs
        for (auto i = 0; i < 8; ++i) {
            // Regardless of IRQ 2
            CXX11_CONSTEXPR
            static auto const kPicCascadeIrqNum = 2;
            if (i != kPicCascadeIrqNum) {
                apic_configure_isa_irq(i, ApicDeliveryMode::Fixed, true, 
                               ApicDestinationMode::Physical, bspid, IrqVec());
            }
            apic_configure_isa_irq(i + 8, ApicDeliveryMode::Fixed, true, 
                                   ApicDestinationMode::Physical, bspid, IrqVec());
        }

        // Initialize previous 16 ISA IRQ.
        irq::init_early(16);
    }
    GKTL_INIT_HOOK(ApicInit, platform_init_apic, gktl::InitLevel::Vmm);

    auto platform_handle_irq(HIrqNum irqnum, arch::IrqFrame *frame) -> void {
        irq::handle_irq_generic(irqnum);
    }

} // namespace ours
