#include <ours/types.hpp>
#include <ours/status.hpp>
#include <ours/cpu_local.hpp>
#include <ours/kernel_entry.hpp>

#include <ours/task/thread.hpp>

#include <logz4/log.hpp>

namespace ours {
    /// Initialize all static life-cycles objects.
    /// Defined in file static_objects.cpp
    auto init_static_objects() -> void;

    static auto labour_routine() -> void
    {
        // Reclaim memories occupied by the early infomation.
    }

    /// Called from arch-code.
    /// Note: Invoke it after finishing the early architecture initialization.
    /// Assumptions:
    ///     1). The early memory allocator has been initialized.
    NO_MANGLE
    auto start_kernel(KernelParam *params) -> Status
    {
        init_static_objects();
        gktl::CpuLocal::init(params->cpuid);

        auto laborer = task::Thread::spawn("laborer", labour_routine);
        laborer->resume();

        return Status::Ok;
    }

    NO_MANGLE
    auto start_nonboot_cpu() -> Status
    {
        // gktl::CpuLocal::init();
        return Status::Ok;
    }
}