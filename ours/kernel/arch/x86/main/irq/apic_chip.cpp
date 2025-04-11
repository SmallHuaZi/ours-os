#include <ours/irq/irq_chip.hpp>
#include <arch/x86/apic/xapic.hpp>
#include <arch/x86/msr.hpp>

namespace ours {
    using irq::IrqChip;
    using irq::IrqData;

    struct XApic: public IrqChip {
        XApic();

        auto startup(IrqData &data) -> void override;
        auto teawdown(IrqData &data) -> void override;
        auto enable(IrqData &data) -> void override;
        auto disable(IrqData &data) -> void override;

        auto mask(IrqData &data) -> void override;
        auto unmask(IrqData &data) -> void override;
        auto send_eoi(IrqData &data) -> void override;
        auto send_ack(IrqData &data) -> void override;
        auto send_ipi(IrqData &data, CpuNum cpu) -> void override;

        arch::XApic impl_;
    };

    XApic::XApic()
        : IrqChip(),
          impl_() {
        name_ = "XApic";
    }

    auto XApic::startup(IrqData &data) -> void {
    }

    auto XApic::teawdown(IrqData &data) -> void {
    }

    auto XApic::enable(IrqData &data) -> void {
    }

    auto XApic::send_ipi(IrqData &data, CpuNum cpu) -> void {
    } 

} // namespace ours