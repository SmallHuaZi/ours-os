#include <ours/task/mod.hpp>

#include <ours/task/scheduler.hpp>
#include <ours/task/sched_object.hpp>
#include <ours/cpu-local.hpp>

namespace ours::task {
    auto schedule() -> void {
        MainScheduler::Current::schedule();
    }

    auto activate_thread(Thread *thread) -> Status {
        MainScheduler::Current::get()->activate_thread(thread);
        return Status::Ok;
    }

    auto deactivate_thread(Thread *thread) -> Status {
        MainScheduler::Current::get()->deactivate_thread(thread);
        return Status::Ok;
    }

} // namespace ours::task