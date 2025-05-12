#include <ours/start.hpp>
#include <ours/init.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/thread.hpp>
#include <ours/sched/scheduler.hpp>

/// Init calls
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/sched/init.hpp>
#include <ours/platform/init.hpp>

#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>

#include <gktl/init_hook.hpp>
#include <gktl/static_objects.hpp>

#include <ours/platform/acpi.hpp>

namespace ours {
    INIT_CODE
    static auto init_first_thread() -> void {
        // We need this dummy thread as `current` thread to make a feint 
        // that there has been a thread which is running.
        static task::Thread dummy{0, "dummy"};

        new (&dummy) decltype(dummy)(0, "dummy");
        sched::MainScheduler::get()->init_thread(dummy, sched::BaseProfile(0));
        sched::MainScheduler::set_current_thread(&dummy);
        task::Thread::Current::set(&dummy);

        // FIXME(SmallHuaZi): There is no a kernel stack for the first thread.
        // Taking the following way will leak mem::Stack::kDefaultStackSize size of memory.
        dummy.kernel_stack().init();
    }

    INIT_CODE
    static auto labour_routine() -> i32 {
        init_arch();
        set_init_level(gktl::InitLevel::Arch);

        init_platform();
        set_init_level(gktl::InitLevel::Platform);

        // Ok, now there is no any routine to access the `init` area.
        mem::reclaim_init_area();

        // Does we need a kernel shell?

        // Load userboot 
        return 0;
    }

    /// Called from arch-code.
    /// Note: Invoke it after finishing the collection to early architecture specific information passed from `PhysBoot`.
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

        sched::init_sched();

        init_first_thread();

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