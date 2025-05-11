#include <ours/ipi-event.hpp>
#include <ours/arch/mp.hpp>

#include <ours/sched/scheduler.hpp>

namespace ours {
    WEAK_LINK
    auto mp_init() -> void {}

    auto mp_reschedule(CpuMask mask, u32 flags) -> void {
        // Clear local CPU and inactive CPUs.
        auto const local_cpu = CpuLocal::cpunum();
        mask.set(local_cpu, 0);
        mask &= sched::MainScheduler::peek_active_mask();

        if (!mask.count()) {
            return;
        }

        arch_mp_reschedule(mask);
    }

    auto mp_interrupt(IpiTarget target, CpuMask mask) -> void {
        arch_mp_send_ipi(target, mask, IpiEvent::Generic);
    }

} // namespace ours