#include <ours/task/timer.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/irq/mod.hpp>

#include <arch/x86/interrupt.hpp>
#include <ustl/chrono/duration.hpp>
#include <gktl/init_hook.hpp>

namespace ours::task {
    CPU_LOCAL
    TimerQueue s_timer_queue;

    auto TimerQueue::tick(CpuNum cpu) -> void {

    }

    auto timer_tick(usize elapsed_time_ms) -> void {
        // We always update scheduling information first.
        CpuLocal::access<sched::MainScheduler>()->tick();

        CpuLocal::access(&s_timer_queue)->tick(arch_current_cpu());
    }

} // namespace ours::task