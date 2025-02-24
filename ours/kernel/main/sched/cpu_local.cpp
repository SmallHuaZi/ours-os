#include <ours/cpu_local.hpp>

#include "main_scheduler.hpp"

#include <ustl/lazy/lazy_init.hpp>

using ours::sched::MainScheduler;
using ustl::lazy::LazyInit;

CPU_LOCAL
static LazyInit<MainScheduler>  LOCAL_SCHEDULER;

template <>
auto CpuLocal::access<MainScheduler>(CpuId cpuid) -> MainScheduler * {
    return Self::access(LOCAL_SCHEDULER.data(), cpuid);
}