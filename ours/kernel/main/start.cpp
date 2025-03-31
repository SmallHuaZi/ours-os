#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu_local.hpp>
#include <ours/task/thread.hpp>
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>

#include <gktl/init_hook.hpp>
#include <gktl/static_objects.hpp>

namespace ours {
    INIT_CODE
    static auto labour_routine() -> void
    {
        // Reclaim memories occupied by the early infomation.
        init_arch();
        set_init_level(gktl::InitLevel::Arch);

        init_platform();
        set_init_level(gktl::InitLevel::Platform);

        // reclaim_init_area(mem::RemTag::All);

        // Load userboot 
    }

    /// Called from arch-code.
    /// Note: Invoke it after finishing the collection to early architecture specific information.
    /// Assumptions:
    ///     1). The early memory allocator has been initialized.
    NO_MANGLE INIT_CODE
    auto start_kernel(PhysAddr handoff) -> Status
    {
        gktl::init_static_objects();
        setup_handoff(handoff);

        init_arch_early();
        set_init_level(gktl::InitLevel::ArchEarly);

        init_platform_early();
        set_init_level(gktl::InitLevel::PlatformEarly);

        // At this point, `PMM` must be initialized.
        CpuLocal::init(0);

        mem::init_vmm();
        set_init_level(gktl::InitLevel::VmmInitialized);

        irq::init_irq();
        set_init_level(gktl::InitLevel::IrqInitialized);

        // Start from here, memory allocator is alive.
        // First thing we should do is to initialize our system logger.
        log::init(0);

        auto const laborer = task::Thread::spawn("laborer", labour_routine);
        laborer->detach();
        laborer->resume();

        return Status::Ok;
    }

    NO_MANGLE
    auto start_nonboot_cpu(CpuNum CpuNum) -> Status
    {
        CpuLocal::init(CpuNum);
        return Status::Ok;
    }
}