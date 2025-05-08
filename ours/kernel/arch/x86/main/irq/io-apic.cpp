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
    static ustl::Array<arch::IoApicIsaOverride, arch::kNumIsaIrqs> s_ioapic_isa_overrides;
    irq::IrqChip *g_ioapic_chip;

    static auto resolve_irqnum_to_ioapic(HIrqNum irqnum) -> IoApic * {
        for (auto i = 0; i < s_ioapics.size(); ++i) {
            if (s_ioapics[i].arch_ioapic().gsi_base() <= irqnum && 
                irqnum < s_ioapics[i].arch_ioapic().max_local_irqnum()) {
                return &s_ioapics[i];
            }
        }

        return 0;
    }

    auto apic_mask_irq(HIrqNum global_irq) -> void {
        s_ioapics[0].inner_.mask_irq(global_irq);
    }

    auto apic_unmask_irq(HIrqNum global_irq) -> void {
        s_ioapics[0].inner_.unmask_irq(global_irq);
    }

    auto apic_configure_irq(HIrqNum global_irq, arch::ApicTriggerMode trimode, 
                            arch::ApicInterruptPolarity polarity, arch::ApicDeliveryMode delmode, 
                            bool mask, arch::ApicDestinationMode dstmode, u8 dst, arch::IrqVec vector) -> void {
        IoApic *ioapic = resolve_irqnum_to_ioapic(global_irq);
        ASSERT(ioapic);

        ioapic->arch_ioapic().config_irq(
            global_irq, 
            trimode, 
            polarity, 
            delmode, 
            dstmode, 
            dst, 
            vector,
            mask
        );

        log::trace("IO-APIC[{}]: configuration({}, {}, {}, {}, {}, {})", 
            ioapic->inner_.id(),
            to_string(trimode),
            to_string(polarity),
            to_string(delmode),
            to_string(dstmode),
            dst,
            usize(vector)
        );
    }

    auto apic_configure_isa_irq(u8 isa_irq, arch::ApicDeliveryMode del_mode, bool mask,
                                arch::ApicDestinationMode dst_mode, u8 dst, arch::IrqVec vector) -> void {
        HIrqNum irq = isa_irq;
        auto trigger_mode = arch::ApicTriggerMode::Edge; 
        auto polarity = arch::ApicInterruptPolarity::ActiveHigh; 
        if (s_ioapic_isa_overrides[isa_irq].remapped) {
            irq = s_ioapic_isa_overrides[isa_irq].global_irq;
            trigger_mode = s_ioapic_isa_overrides[isa_irq].tri_mode;
            polarity = s_ioapic_isa_overrides[isa_irq].polarity;
        }

        apic_configure_irq(irq, trigger_mode, polarity, del_mode, mask, dst_mode, dst, vector);
    }

    INIT_CODE
    auto init_io_apic(ustl::views::Span<arch::IoApic> const &ioapics,
                      ustl::views::Span<arch::IoApicIsaOverride> const &overrides) -> void {
        auto const nr_ioapics = ioapics.size();
        auto const nr_overrides = overrides.size();
        {
            auto raw_ioapics = new (mem::kGafKernel) IoApic[nr_ioapics]();
            ASSERT(raw_ioapics, "Failed to allocate IO-APIC");
            s_ioapics = ustl::views::make_span(raw_ioapics, nr_ioapics);
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

        log::info("ISA Irq | Remapped | Trigger mode | Polarity | Global IRQ");
        for (auto i = 0; i < nr_overrides; ++i) {
            /// Make a linear mapping
            s_ioapic_isa_overrides[overrides[i].isa_irq] = overrides[i];

            log::trace("{: <6} | {} | {} | {} | {}", 
                s_ioapic_isa_overrides[i].isa_irq, 
                s_ioapic_isa_overrides[i].remapped, 
                to_string(s_ioapic_isa_overrides[i].tri_mode),
                to_string(s_ioapic_isa_overrides[i].polarity),
                s_ioapic_isa_overrides[i].global_irq
            );
        }

        g_ioapic_chip = &s_ioapics[0];
    }

} // namespace ours