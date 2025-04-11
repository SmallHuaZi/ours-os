#include <ours/platform/init.hpp>
#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/thread.hpp>
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>

#include <gktl/init_hook.hpp>
#include <gktl/static_objects.hpp>

#include <ours/platform/acpi.hpp>

namespace ours {
    INIT_CODE
    static auto labour_routine() -> void {
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
    auto start_kernel(PhysAddr handoff) -> void {
        gktl::init_static_objects();

        // Install early cpu local data.
        CpuLocal::init_early();

        setup_handoff(handoff);

        // Start from here, memory allocator is alive.
        // First thing we should do is to initialize our system logger.
        log::init(0);

        init_arch_early();
        set_init_level(gktl::InitLevel::ArchEarly);

        init_platform_early();
        set_init_level(gktl::InitLevel::PlatformEarly);

        // At this point, `PMM` must be initialized.
        CpuLocal::init(arch_current_cpu());

        mem::init_vmm();
        set_init_level(gktl::InitLevel::VmmInitialized);

        irq::init_irq();
        set_init_level(gktl::InitLevel::IrqInitialized);

        auto const laborer = task::Thread::spawn("laborer", labour_routine);
        laborer->detach();
        laborer->resume();

        // Let the main thread become the idle thread. 
        task::Thread::Current::idle();
    }

    NO_MANGLE
    auto start_nonboot_cpu() -> Status {
        CpuLocal::init_percpu();
        return Status::Ok;
    }
}