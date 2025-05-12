#include <ours/platform/init.hpp>
#include <ours/platform/acpi.hpp>
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>
#include <ours/cpu-states.hpp>
#include <ours/arch/mp.hpp>

#include <arch/x86/pic.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void {
        set_platform_init_level(PlatformInitLevel::StartUp);

        set_platform_init_level(PlatformInitLevel::PrePmm);

        platform_init_pmm();
        set_platform_init_level(PlatformInitLevel::Pmm);
    }

    FORCE_INLINE
    static auto platform_init_mp() -> void {
        x86_wakeup_aps(global_cpu_states().possible_cpus);
    }

    NO_MANGLE INIT_CODE
    auto init_platform() -> void {
        platform_init_mp();
    }

} // namespace ours