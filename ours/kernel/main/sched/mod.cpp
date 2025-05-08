#include <ours/sched/mod.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>
#include <ours/cpu-local.hpp>

namespace ours::sched {
    auto reschedule() -> void {
        MainScheduler::current() ->reschedule();
    }

} // namespace ours::sched