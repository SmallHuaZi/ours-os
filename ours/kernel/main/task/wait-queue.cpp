#include <ours/task/wait-queue.hpp>

#include <ours/task/thread.hpp>
#include <ours/sched/scheduler.hpp>

namespace ours::task {
    auto Waiter::wait(bool interruptible, Status status) -> void {
        auto thread = Thread::of(this);

        status_ = status;
        thread->set_interruptible();
        sched::MainScheduler::block(*thread);
        thread->clear_interruptible();
    }

    auto Waiter::notify(Status status) -> void {
        auto thread = Thread::of(this);
        status_ = status;
        sched::MainScheduler::unblock(*thread);
    }

} // namespace ours::task