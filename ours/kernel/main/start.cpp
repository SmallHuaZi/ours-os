#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/thread.hpp>
#include <ours/task/scheduler.hpp>

/// Init calls
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/task/init.hpp>
#include <ours/platform/init.hpp>

#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>

#include <gktl/init_hook.hpp>
#include <gktl/static_objects.hpp>

#include <ours/platform/acpi.hpp>

namespace ours {
    INIT_CODE
    static auto labour_routine() -> i32 {
        init_arch();
        set_init_level(gktl::InitLevel::Arch);

        init_platform();
        set_init_level(gktl::InitLevel::Platform);

        // Ok, now there is no any routine to access the `init` area.
        mem::reclaim_init_area();

        // Does we need a kernel shell?

        // The 2nd stage has finished, and next is to load userboot.
        return 0;
    }

    /// Called from arch-code.
    /// Note: Invoke it after finishing the collection to early architecture 
    /// specific information passed from `PhysBoot`.
    ///
    /// Assumptions:
    ///     1). The early memory allocator has been initialized.
    NO_MANGLE INIT_CODE
    auto start_kernel(PhysAddr handoff) -> void {
        gktl::init_static_objects();

        setup_handoff(handoff);

        // Install early cpu local data. The dynamic cpu local allocator for BP will be installed. 
        CpuLocal::init_early();

        init_arch_early();
        set_init_level(gktl::InitLevel::ArchEarly);

        init_platform_early();
        set_init_level(gktl::InitLevel::PlatformEarly);

        mem::init_vmm();
        set_init_level(gktl::InitLevel::Vmm);

        CpuLocal::init();
        set_init_level(gktl::InitLevel::CpuLocal);

        // TODO(SmallHuaZi): It seems like doing nothing. Consider to remove
        // it or give it more thing to do.
        irq::init_irq();
        set_init_level(gktl::InitLevel::Irq);

        task::init_task();

        auto const laborer = task::Thread::spawn("laborer", 0, labour_routine);
        laborer->detach();
        laborer->resume();  // At this point, the first thread is running.

        // Let the main thread become the idle thread. 
        task::Thread::Current::idle();
    }

    NO_MANGLE
    auto start_nonboot_cpu() -> Status {
        return Status::Ok;
    }
}