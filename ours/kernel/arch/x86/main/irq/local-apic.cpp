#include <ours/arch/apic.hpp>
#include <ours/irq/irq_chip.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_mapping.hpp>
#include <ours/cpu-local.hpp>

#include <arch/x86/apic/xapic.hpp>
#include <arch/x86/msr.hpp>
#include <arch/x86/interrupt.hpp>

#include <logz4/log.hpp>

namespace ours::irq {
    static void *s_apic_mmio_base;
    static bool s_x2apic_enabled = false;

    CPU_LOCAL
    static u32 s_apic_id;

    auto current_apic_id() -> u32 {
        return CpuLocal::read(s_apic_id);
    }

    struct XApic: public IrqChip {
        XApic();

        auto startup(IrqData &data) -> void override;
        auto teawdown(IrqData &data) -> void override;

        auto mask(IrqData &data) -> void override;
        auto unmask(IrqData &data) -> void override;
        auto send_eoi(IrqData &data) -> void override;
        auto send_ack(IrqData &data) -> void override;
        auto send_ipi(IrqData &data, CpuNum cpu) -> void override;

        arch::XApic inner_;
    };

    XApic::XApic()
        : IrqChip(),
          inner_() {
        name_ = "XApic";
    }

    auto XApic::startup(IrqData &data) -> void {
    }

    auto XApic::teawdown(IrqData &data) -> void {
    }

    auto XApic::send_ipi(IrqData &data, CpuNum cpu) -> void {
    } 

    auto XApic::mask(IrqData &data) -> void {
    } 

    auto XApic::unmask(IrqData &data) -> void {
    } 

    auto XApic::send_eoi(IrqData &data) -> void {
        inner_.send_eoi();
    } 

    auto XApic::send_ack(IrqData &data) -> void {
        inner_.send_eoi();
    }

    static XApic s_xapic_chip;

    INIT_CODE
    auto init_local_apic() -> void {
        using namespace mem;
        auto rvma = VmAspace::kernel_aspace()->root_vma();
        auto result = rvma->map_at(arch::kApicPhysBase, 0, PAGE_SIZE, 
            // `Discache` is required to prevent 
            MmuFlags::Discache | MmuFlags::Readable | MmuFlags::Writable, 
            VmMapOption::Commit | VmMapOption::Pinned, "Local-APIC"
        );
        DEBUG_ASSERT(result, "Failed to allocate MMIO for BP's Local APIC");

        auto mmio_base(reinterpret_cast<u32 *>((*result)->base()));

        s_xapic_chip.inner_.init(mmio_base);

        CpuLocal::write(s_apic_id, s_xapic_chip.inner_.id());

        init_local_apic_percpu();
    }

    INIT_CODE
    static auto init_apic_timer() -> void {
        s_xapic_chip.inner_.enable_tsc(arch::IrqVec::ApicTimer);
        s_xapic_chip.inner_.mask(arch::XApicRegType::LvtTimer);

        if (x86_has_feature(CpuFeatureType::TscDeadlineTimer)) {
            // Stub
        }
    }

    INIT_CODE
    static auto init_apic_pmi() -> void {
        using namespace arch;
        s_xapic_chip.inner_.write_reg(XApicRegType::LvtPmi, IrqVec::ApicPmi);
        s_xapic_chip.inner_.mask(arch::XApicRegType::LvtPmi);
    }

    INIT_CODE
    static auto init_apic_error() -> void {
        using namespace arch;
        s_xapic_chip.inner_.write_reg(XApicRegType::LvtError, IrqVec::ApicError);
        s_xapic_chip.inner_.write_reg(XApicRegType::Esr, 0);
    }

    INIT_CODE
    auto init_local_apic_percpu() -> void {
        using namespace arch;
        auto apic_base = MsrIo::read<usize>(MsrRegAddr::IA32ApicBase);
        if (apic_base & arch::kIa32ApicBaseBsp) {
            if (x86_has_feature(CpuFeatureType::X2Apic)) {
                log::trace("X2-APIC enabled");
            }
        }

        apic_base |= arch::kIa32ApicBaseXapicEnable;
        if (s_x2apic_enabled) {
            apic_base |= arch::kIa32ApicBaseX2ApicEnable;
        }

        // Enable X[2]-APIC
        MsrIo::write<usize>(MsrRegAddr::IA32ApicBase, apic_base);

        init_apic_timer();
        init_apic_error();
        init_apic_pmi();
    }

} // namespace ours::irq