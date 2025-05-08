#include <ours/platform/init.hpp>
#include <ours/platform/acpi.hpp>
#include <ours/mem/init.hpp>
#include <ours/irq/init.hpp>

#include <arch/x86/pic.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void {
        set_platform_init_level(PlatformInitLevel::StartUp);

        set_platform_init_level(PlatformInitLevel::PrePmm);

        platform_init_pmm();
        set_platform_init_level(PlatformInitLevel::Pmm);
    }

    NO_MANGLE INIT_CODE
    auto init_platform() -> void
    {}

} // namespace ours