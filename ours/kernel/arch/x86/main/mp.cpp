#include <ours/arch/mp.hpp>

#include <ours/arch/x86/init.hpp>
#include <ours/arch/apic.hpp>
#include <ours/arch/x86/idt.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/x86/entry.hpp>
#include <ours/arch/x86/bootstrap.hpp>
#include <ours/platform/timer.hpp>

#include <ours/start.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/thread.hpp>

#include <arch/system.hpp>
#include <arch/halt.hpp>
#include <arch/macro/system.hpp>

using namespace arch;

namespace ours {
    auto arch_mp_send_ipi(IpiTarget target, CpuMask mask, IpiEvent event) -> void {
        IrqVec vector;
        switch (event) {
            case IpiEvent::Generic:
                vector = IrqVec::IpiGeneric;
                break;
            case IpiEvent::Resched:
                vector = IrqVec::IpiResched;
                break;
            case IpiEvent::Interrupt:
                vector = IrqVec::IpiInterrupt;
                break;
            case IpiEvent::Suspend:
                vector = IrqVec::IpiSuspend;
                break;
            default:
                unreachable();
        }

        switch (target) {
            case IpiTarget::Mask:
                apic_send_ipi_mask(mask, vector, ApicDeliveryMode::Fixed);
                break;
            case IpiTarget::All:
                apic_broadcast_ipi(vector, ApicDeliveryMode::Fixed);
                break;
            case IpiTarget::Self:
                apic_send_ipi_self(vector, ApicDeliveryMode::Fixed);
                break;
            default:
                unreachable();
        }
    }

    auto arch_mp_reschedule(CpuMask targets) -> void {
        arch_mp_send_ipi(IpiTarget::Mask, targets, IpiEvent::Resched);
    }

    auto arch_mp_suspend(CpuMask targets) -> void {
        arch_mp_send_ipi(IpiTarget::Mask, targets, IpiEvent::Suspend);
    }

    auto x86_handle_ipi_generic() -> void {}

    auto x86_handle_ipi_interrupt() -> void {}

    auto x86_handle_ipi_resched() -> void {
        auto const this_cpu = CpuLocal::cpunum();
        log::trace("IPI-Resched on CPU[{}]", this_cpu);

        task::Thread::Current::preemption_state().set_pending();
    }

    auto x86_handle_ipi_suspend() -> void {
        auto const this_cpu = CpuLocal::cpunum();
        log::trace("IPI-Suspend on CPU[{}]", this_cpu);

        while (1) {
            arch::disable_interrupt();
            arch::suspend();
        }
    }

    auto arch_activate_cpu(CpuNum) -> Status {  
        return Status::Unimplemented;
    }

    auto arch_deactivate_cpu(CpuNum) -> Status {  
        return Status::Unimplemented;
    }

    NO_MANGLE INIT_CODE
    auto x86_start_nonboot_cpu() -> void {
        auto const cpunum = apic_id_to_cpunum(current_apic_id_early());
        DEBUG_ASSERT(cpunum != kInvalidCpuNum);

        set_current_cpu_online(true);

        CpuLocal::init_percpu(cpunum);

        init_local_apic_percpu();

        x86_init_percpu(CpuLocal::cpunum());

        start_nonboot_cpu();

        // Returning back to here means an error happened.
        // So disable interrupts and suspend. 
        arch::disable_interrupt();
        arch::suspend();
    }

    auto x86_wakeup_aps(CpuMask targets) -> void {
        using task::Thread;
        // Prevent error operation.
        targets.set(0, false);

        for_each_cpu(targets, [] (CpuNum cpu) {
            log::trace("Attemp to wake up CPU[{}]", cpu);
            apic_send_ipi(cpu, arch::IrqVec(), ApicDeliveryMode::Init);
        });

        BootstrapData *bootstrap_data;
        auto const trampoline = make_bootstrap_area(&bootstrap_data);
        DEBUG_ASSERT(trampoline < MB(1));

        auto now = current_mono_time();
        log::trace("CPU[0]-time={}ns", now);

        Thread::Current::sleep_for(Milliseconds(10), true);

        for_each_cpu(targets, [bootstrap_data, trampoline] (CpuNum cpu) {
            log::trace("Attemp to wake up CPU[{}]", cpu);

            bootstrap_data->this_cpu = cpu;
            apic_send_ipi(cpu, arch::IrqVec(trampoline >> PAGE_SHIFT), ApicDeliveryMode::Startup);


            Thread::Current::sleep_for(Milliseconds(1), true);
        });
    }

} // namespace ours