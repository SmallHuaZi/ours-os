#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/thread.hpp>

/// Init calls
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/platform/init.hpp>

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
    /// Note: Invoke it after finishing the collection to early architecture specific information passed from `PhysBoot`.
    /// Assumptions:
    ///     1). The early memory allocator has been initialized.
    NO_MANGLE INIT_CODE
    auto start_kernel(PhysAddr handoff) -> void {
        gktl::init_static_objects();

        // Install early cpu local data. The dynamic cpu local allocator for BP will be installed. 
        CpuLocal::init_early();

        setup_handoff(handoff);

        init_arch_early();
        set_init_level(gktl::InitLevel::ArchEarly);

        init_platform_early();
        set_init_level(gktl::InitLevel::PlatformEarly);

        // Before here, the PMM should be fully initialied.
        CpuLocal::init();
        set_init_level(gktl::InitLevel::CpuLocal);

        mem::init_vmm();
        set_init_level(gktl::InitLevel::Vmm);

        irq::init_irq();
        set_init_level(gktl::InitLevel::Irq);

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