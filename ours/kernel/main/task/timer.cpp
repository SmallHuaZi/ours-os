#include <ours/task/timer.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/irq/mod.hpp>

#include <arch/x86/interrupt.hpp>
#include <ustl/chrono/duration.hpp>
#include <gktl/init_hook.hpp>

namespace ours::task {
    CPU_LOCAL
    TimerQueue TimerQueue::s_timer_queue;

    auto TimerQueue::tick() -> void {

    }

    auto timer_tick(usize elapsed_time_ms) -> void {
        // We always update scheduling information first.
        sched::MainScheduler::current()->tick();

        TimerQueue::current()->tick();
    }

} // namespace ours::task