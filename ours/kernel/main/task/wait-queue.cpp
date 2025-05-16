#include <ours/task/wait-queue.hpp>

#include <ours/task/thread.hpp>
#include <ours/task/scheduler.hpp>

namespace ours::task {
    auto WaiterState::wait(bool interruptible, Status status) -> void {
        auto thread = Thread::of(this);

        status_ = status;
        thread->set_interruptible();
        // MainScheduler::Current::deactivate_thread(thread);
        thread->clear_interruptible();
    }

    auto WaiterState::notify(Status status) -> void {
        auto thread = Thread::of(this);
        status_ = status;
        // MainScheduler::deactivate_thread(thread);
    }

} // namespace ours::task