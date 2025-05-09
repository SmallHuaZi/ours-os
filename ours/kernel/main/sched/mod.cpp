#include <ours/sched/mod.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>
#include <ours/cpu-local.hpp>

namespace ours::sched {
    auto reschedule() -> void {
        MainScheduler::current() ->reschedule();
    }

    auto activate_thread(task::Thread *thread) -> void {
        MainScheduler::current()->activate(*thread);
    }

    auto deactivate_thread(task::Thread *thread) -> void {
        MainScheduler::current()->deactivate(*thread);
    }

} // namespace ours::sched