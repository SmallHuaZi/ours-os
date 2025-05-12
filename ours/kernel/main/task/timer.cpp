#include <ours/task/timer.hpp>
#include <ours/task/timer-queue.hpp>

namespace ours::task {
    auto Timer::activate(Deadline deadline, OnExpired on_expired) -> void {
        canary_.verify();

        deadline_ = deadline.latest();
        on_expired_ = on_expired;
        cancalled_.store(false, ustl::sync::MemoryOrder::Release);

        TimerQueue::current()->insert_timer(*this, deadline.earliest(), deadline.latest());
    }

    auto Timer::cancel() -> void {
        cancalled_.store(true, ustl::sync::MemoryOrder::Release);
    }

} // namespace ours::task