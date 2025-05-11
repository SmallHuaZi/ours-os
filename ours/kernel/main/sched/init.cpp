#include <ours/sched/init.hpp>

#include <ours/sched/scheduler.hpp>

namespace ours::sched {
    auto init_sched() -> void {
        sched::MainScheduler::get()->init();
    }

} // namespace ours::sched