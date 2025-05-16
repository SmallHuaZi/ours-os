#include <ours/task/timer-queue.hpp>

#include <ours/task/scheduler.hpp>
#include <ours/irq/mod.hpp>
#include <ours/platform/timer.hpp>

#include <arch/interrupt.hpp>
#include <ustl/chrono/duration.hpp>
#include <gktl/init_hook.hpp>

namespace ours::task {
    auto TimerQueue::convert_mono_time_to_ticks(TimePoint deadline) -> Ticks {
        return platform_convert_mono_time_to_ticks(deadline.time_since_epoch().count());
    }

    auto TimerQueue::reset_preemption_timer(TimePoint deadline) -> void {
        preemption_timer_deadline_ = deadline;
        update_mono_deadline(deadline);
    }

    auto TimerQueue::update_mono_deadline(TimePoint deadline) -> void {
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
        if (!timer_list_.empty()) {
            deadline = min(deadline, timer_list_.front().deadline());
        }

        MonoTicks deadline_ticks = convert_mono_time_to_ticks(deadline);
        platform_set_oneshot_timer(deadline_ticks);
        next_timer_deadline_ = deadline_ticks;
    }

    auto TimerQueue::insert_timer(Timer &timer, TimePoint earliest, TimePoint latest) -> void {
        auto iter = timer_list_.begin();
        for (auto last = timer_list_.end(); iter != last; ++iter) {
            if (iter->deadline() > timer.deadline()) {
                break;
            }
        }
        timer_list_.insert(iter, timer);
        update_deadline();
    }

    auto TimerQueue::tick(CpuNum cpu) -> void {
        canary_.verify();

        auto const now = current_mono_time();
        log::trace("CPU[{}]-time={} ns", cpu, now);

        // Tickless
        if (now >= preemption_timer_deadline_.time_since_epoch().count()) {
            preemption_timer_deadline_ = kInfiniteTime;
            MainScheduler::tick(SchedTime(now));
        }
        next_timer_deadline_ = kInfiniteTicks;

        while (!timer_list_.empty()) {
            auto &timer = timer_list_.front();
            if (timer.deadline().time_since_epoch().count() > now) {
                break;
            }

            timer_list_.erase(timer_list_.iterator_to(timer));
            timer.on_expired();
        }

        update_deadline();
    }

    auto timer_tick() -> void {
        // We always update scheduling information first.
        auto const this_cpu = CpuLocal::cpunum();
        TimerQueue::get()->tick(this_cpu);
    }

} // namespace ours::task