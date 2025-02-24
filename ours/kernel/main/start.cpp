#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu_local.hpp>
#include <ours/task/thread.hpp>
#include <ours/mem/init.hpp>
#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>

#include <gktl/init_hook.hpp>
#include <gktl/static_objects.hpp>

namespace ours {
    static auto labour_routine() -> void
    {
        // Reclaim memories occupied by the early infomation.
        init_arch();

        init_platform();
    }

    /// Called from arch-code.
    /// Note: Invoke it after finishing the early architecture initialization.
    /// Assumptions:
    ///     1). The early memory allocator has been initialized.
    NO_MANGLE
    auto start_kernel(PhysAddr handoff) -> Status
    {
        gktl::init_static_objects();
        setup_handoff(handoff);

        // Set up early memory manager.
        mem::init_early_pmm(PHYS_HANDOFF->mem);

        CpuLocal::init(BOOT_CPU_ID);

        init_arch_early();
        gktl::set_init_level(gktl::InitLevel::ArchEarly);

        init_platform_early();
        gktl::set_init_level(gktl::InitLevel::PlatformEarly);

        mem::init_vmm();
        gktl::set_init_level(gktl::InitLevel::VmmInitialized);

        // Start from here, memory allocator is alive.
        // First thing we should do is to initialize our system logger.
        log::init();

        auto laborer = task::Thread::spawn("laborer", labour_routine);
        laborer->detach();
        laborer->resume();

        return Status::Ok;
    }

    NO_MANGLE
    auto start_nonboot_cpu(CpuId cpuid) -> Status
    {
        CpuLocal::init(cpuid);
        return Status::Ok;
    }
}