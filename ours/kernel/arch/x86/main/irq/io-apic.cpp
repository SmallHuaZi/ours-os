#include <ours/arch/apic.hpp>
#include <ours/irq/irq_chip.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <arch/x86/apic/ioapic.hpp>
#include <arch/intr_disable_guard.hpp>

#include <ktl/new.hpp>

namespace ours::irq {
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
        
        auto startup(IrqData &data) -> void override;
        auto teawdown(IrqData &data) -> void override;
        auto enable(IrqData &data) -> void override;
        auto disable(IrqData &data) -> void override;

        auto mask(IrqData &data) -> void override;
        auto unmask(IrqData &data) -> void override;
        auto ack(IrqData &data) -> void override;
        auto send_eoi(IrqData &data) -> void override;
        auto send_ack(IrqData &data) -> void override;
        auto send_ipi(IrqData &data, CpuNum cpu) -> void override;

        arch::IoApic inner_;
    };

    IoApic::IoApic(arch::IoApic inner)
        : inner_(inner)
    {}

    auto IoApic::send_eoi(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.issue_eoi(data.virqnum);
    }

    auto IoApic::mask(IrqData &data) -> void {
        arch::IntrDisableGuard guard;
        inner_.mask_irq(data.virqnum);
    }

    static ustl::views::Span<IoApic> s_ioapics;

    INIT_CODE
    auto init_io_apic(ustl::views::Span<arch::IoApic> &ioapics,
                      ustl::views::Span<arch::IoApicIsaOverride> &overrides) -> void {
        auto const nr_ioapics = ioapics.size();
        {
            auto raw = new (mem::kGafKernel) IoApic[nr_ioapics]();
            ASSERT(raw, "Failed to allocate IO-APIC");
            s_ioapics = ustl::views::make_span(raw, nr_ioapics);
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

            if (!mmio_virt) {
                // No other shared IO-APICs, we have to create a new mapping for it.
                auto rvma = mem::VmAspace::kernel_aspace()->root_vma();
            }

            s_ioapics[i].arch_ioapic().reset_mmio(mmio_virt);
        }
    }

} // namespace ours::irq