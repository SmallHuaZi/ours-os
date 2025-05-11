#include <ours/task/timer.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/irq/mod.hpp>
#include <ours/platform/timer.hpp>

#include <arch/interrupt.hpp>
#include <ustl/chrono/duration.hpp>
#include <gktl/init_hook.hpp>

namespace ours::task {
    auto TimerQueue::convert_mono_time_to_ticks(MonoInstant deadline) -> Ticks {
        return platform_convert_mono_time_to_ticks(deadline);
    }

    auto TimerQueue::reset_preemption_timer(MonoInstant deadline) -> void {
        preemption_timer_deadline_ = deadline;
        update_mono_deadline(deadline);
    }

    auto TimerQueue::update_mono_deadline(MonoInstant deadline) -> void {
        DEBUG_ASSERT(arch::interrupt_disabled());

        if (deadline == kInfiniteTime) {
            return;
        }

        auto const deadline_ticks = convert_mono_time_to_ticks(deadline);
        if (deadline_ticks < next_timer_deadline_) {
            next_timer_deadline_ = deadline_ticks;
            platform_set_oneshot_timer(next_timer_deadline_);
        }
    }

    auto TimerQueue::update_deadline() -> void {
        using ustl::algorithms::min;

        auto deadline = preemption_timer_deadline_;
        if (!mono_timer_list_.empty()) {
            deadline = min(deadline, mono_timer_list_.front().deadline());
        }

        MonoTicks deadline_ticks = convert_mono_time_to_ticks(deadline);
        platform_set_oneshot_timer(deadline_ticks);
        next_timer_deadline_ = deadline;
    }

    auto TimerQueue::tick(CpuNum cpu) -> void {
        canary_.verify();

        auto const now = current_mono_time();
        log::trace("CPU[{}]-time={} ns", cpu, now);

        if (now >= preemption_timer_deadline_) {
            preemption_timer_deadline_ = kInfiniteTime;
            sched::MainScheduler::tick(sched::SchedTime(now));
        }
        next_timer_deadline_ = kInfiniteTime;

        update_deadline();
    }

    auto timer_tick(usize elapsed_time_ms) -> void {
        // We always update scheduling information first.
        auto const this_cpu = CpuLocal::cpunum();
        TimerQueue::current()->tick(this_cpu);
    }

} // namespace ours::task