#include <ours/cpu_local.hpp>

#include "main_scheduler.hpp"

#include <ustl/lazy_init.hpp>

namespace ours {
    CPU_LOCAL
    static ustl::LazyInit<sched::MainScheduler>  LOCAL_SCHEDULER;

    template <>
    auto CpuLocal::access<sched::MainScheduler>(CpuNum CpuNum) -> sched::MainScheduler * {
        return Self::access(LOCAL_SCHEDULER.data(), CpuNum);
    }

} // namespace ours