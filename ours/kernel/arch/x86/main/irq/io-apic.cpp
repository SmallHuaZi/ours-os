#include <ours/arch/apic.hpp>
#include <ours/irq/irq_chip.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_mapping.hpp>

#include <arch/x86/apic/ioapic.hpp>
#include <arch/intr_disable_guard.hpp>

#include <ustl/mem/box.hpp>
#include <ktl/new.hpp>
#include <logz4/log.hpp>

namespace ours {
    using namespace irq;

    class IoApic: public IrqChip {
        typedef IoApic      Self;
        typedef IrqChip     Base;
      public:
        IoApic() = default;
        IoApic(arch::IoApic);
        virtual ~IoApic() = default;

        FORCE_INLINE
        auto arch_ioapic() -> arch::IoApic & {
            return inner_;
        }
        
        auto startup(IrqData &data) -> void override {};
        auto teawdown(IrqData &data) -> void override {};
        auto enable(IrqData &data) -> void override {};
        auto disable(IrqData &data) -> void override {};

        auto mask(IrqData &data) -> void override;
        auto unmask(IrqData &data) -> void override;
        auto send_eoi(IrqData &data) -> void override;
        auto send_ack(IrqData &data) -> void override;

        auto dump() const -> void;

        arch::IoApic inner_;
    };

    IoApic::IoApic(arch::IoApic inner)
        : inner_(inner)
    {}

    auto IoApic::send_eoi(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.issue_eoi(data.virqnum);
    }

    auto IoApic::send_ack(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.issue_eoi(data.virqnum);
    }

    auto IoApic::mask(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.mask_irq(data.virqnum);
    }

    auto IoApic::unmask(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.unmask_irq(data.virqnum);
    }

    auto IoApic::dump() const -> void {
        log::info("IoApic[{}]:", inner_.id());
        log::info("  version: {}", inner_.version());
        log::info("  Addr: 0x{:X}", inner_.phys_base());
        log::info("  MMIO: 0x{:X}", inner_.virt_base());
        log::info("  GBI base: {}", inner_.gsi_base());
        log::info("  Max local IRQ: {}", inner_.max_local_irqnum());
    }

    static ustl::views::Span<IoApic> s_ioapics;
    static ustl::views::Span<arch::IoApicIsaOverride> s_ioapic_isa_overrides;

    INIT_CODE
    auto init_io_apic(ustl::views::Span<arch::IoApic> const &ioapics,
                      ustl::views::Span<arch::IoApicIsaOverride> const &overrides) -> void {
        auto const nr_ioapics = ioapics.size();
        auto const nr_overrides = overrides.size();
        {
            auto raw_ioapics = new (mem::kGafKernel) IoApic[nr_ioapics]();
            ASSERT(raw_ioapics, "Failed to allocate IO-APIC");
            s_ioapics = ustl::views::make_span(raw_ioapics, nr_ioapics);

            auto raw_overrides = new (mem::kGafKernel) arch::IoApicIsaOverride[nr_overrides]();
            ASSERT(raw_overrides, "Failed to allocate IO-APIC");
            s_ioapic_isa_overrides = ustl::views::make_span(raw_overrides, nr_overrides);
        }

        for (auto i = 0; i < nr_ioapics; ++i) {
            // Inplacement new to re-construct IO-APIC descriptor.
            s_ioapics[i].arch_ioapic() = ioapics[i];

            // Then need we create MMIO for per IoApic
            VirtAddr mmio_virt = 0;
            for (auto j = 0; j < i; ++i) {
                if (s_ioapics[i].arch_ioapic().phys_base() ==
                    s_ioapics[j].arch_ioapic().phys_base()) {
                    mmio_virt = s_ioapics[j].arch_ioapic().virt_base();
                }
            }

            // If no other shared IO-APICs, we have to create a new mapping for it.
            if (!mmio_virt) {
                auto const phys = s_ioapics[i].inner_.phys_base();
                auto const phys_aligned = ustl::mem::align_down(phys, PAGE_SIZE);

                using namespace mem;
                auto rvma = VmAspace::kernel_aspace()->root_vma();
                auto result = rvma->map_at(phys_aligned, 0, PAGE_SIZE, 
                    // `Discache` is required to prevent 
                    MmuFlags::Discache | MmuFlags::Readable | MmuFlags::Writable, 
                    VmMapOption::Commit | VmMapOption::Pinned, "IO-APIC"
                );
                DEBUG_ASSERT(result, "Failed to initialize IO-APIC");

                mmio_virt = (*result)->base() + (phys - phys_aligned);
            }

            s_ioapics[i].arch_ioapic().reset_mmio(mmio_virt);
            s_ioapics[i].dump();
        }

        for (auto i = 0; i < nr_overrides; ++i) {
            s_ioapic_isa_overrides[i] = overrides[i];
        }
    }

    auto apic_configure_irq(HIrqNum global_irq, arch::ApicTriggerMode trimode, 
                            arch::ApicInterruptPolarity polarity, arch::ApicDeliveryMode delmode, 
                            bool mask, arch::ApicDestinationMode dstmode, u8 dst, arch::IrqVec vector) -> void {
        IoApic *ioapic;
        ioapic->arch_ioapic().config_irq(global_irq, trimode, polarity, delmode, dstmode, dst, vector);
        if (mask) {
            ioapic->arch_ioapic().mask_irq(global_irq);
        }
    }

    auto apic_configure_isa_irq(u8 isa_irq, arch::ApicDeliveryMode del_mode, bool mask,
                                arch::ApicDestinationMode dst_mode, u8 dst, arch::IrqVec vector) -> void {
        HIrqNum irq = isa_irq;
        arch::ApicTriggerMode trigger_mode; 
        arch::ApicInterruptPolarity polarity; 
        if (s_ioapic_isa_overrides[isa_irq].remapped) {
            irq = s_ioapic_isa_overrides[isa_irq].global_irq;
            trigger_mode = s_ioapic_isa_overrides[isa_irq].tri_mode;
            polarity = s_ioapic_isa_overrides[isa_irq].polarity;
        }

        apic_configure_irq(irq, trigger_mode, polarity, del_mode, mask, dst_mode, dst, vector);
    }

} // namespace ours