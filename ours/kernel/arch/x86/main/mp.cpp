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

#include <ustl/fmt/format_to.hpp>

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

    NO_MANGLE NO_RETURN INIT_CODE
    auto x86_start_nonboot_cpu(task::Thread *thread, CpuNum cpunum) -> void {
        DEBUG_ASSERT(cpunum != kInvalidCpuNum);

        CpuLocal::init_percpu(cpunum);

        set_current_cpu_online(true);

        init_local_apic_percpu();

        x86_init_percpu(cpunum);

        start_nonboot_cpu();

        // Returning back to here means an error happened.
        // So disable interrupts and suspend. 
        while (1) {
            arch::disable_interrupt();
            arch::suspend();
        }
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
        auto const vec = arch::IrqVec(trampoline >> PAGE_SHIFT);

        for_each_cpu(targets, [bootstrap_data, vec] (CpuNum cpu) {
            char name[32];
            auto end = ustl::fmt::format_to(name, "Idle-{}", cpu);
            DEBUG_ASSERT(MAX_CPU < 1000'000'000'000'000'000);
            *end = 0;

            auto thread = Thread::spawn(name, 0, [] () -> i32 {
                while (1);
            });

            // FIXME(SmallHuaZi): Only debugging period it is feasible.
            DEBUG_ASSERT(thread); 
            bootstrap_data->pcpu[cpu - 1].current_thread = thread;
            bootstrap_data->pcpu[cpu - 1].stack_top = thread->kernel_stack().top();

            log::trace("CPU[{}]=(s: {:X}, t: {:X})", cpu, 
                bootstrap_data->pcpu[cpu - 1].stack_top,
                VirtAddr(bootstrap_data->pcpu[cpu - 1].current_thread)
            );
        });

        for_each_cpu(targets, [bootstrap_data, vec] (CpuNum cpu) {
            log::trace("Attemp to wake up CPU[{}]", cpu);

            auto &state = global_cpu_states().online_cpus;

            bootstrap_data->cpunum = cpu;
            apic_send_ipi(cpu, vec, ApicDeliveryMode::Startup);

            arch::disable_interrupt();

            // Waiting current thread initialized.
            while (!state.test(cpu))
            {}

            Thread::Current::sleep_for(Milliseconds(1), true);
        });
    }

} // namespace ours