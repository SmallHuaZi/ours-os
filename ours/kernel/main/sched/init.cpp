#include <ours/sched/init.hpp>
#include <ours/sched/scheduler.hpp>

#include <ours/task/timer-queue.hpp>

namespace ours::sched {
    auto init_sched() -> void {
        MainScheduler::get()->init();

        auto tq = task::TimerQueue::current();
        new (tq) task::TimerQueue();
        log::trace("CPU[0].timer_queue at {:X}", VirtAddr(tq));
    }

} // namespace ours::sched