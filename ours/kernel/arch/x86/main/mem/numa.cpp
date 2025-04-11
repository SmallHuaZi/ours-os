#include <ours/platform/init.hpp>
#include <ours/platform/acpi.hpp>
#include <ours/start.hpp>

namespace ours {
    static bool s_numa_enabled = true;

    WEAK_LINK
    auto pci_init_numa() -> Status {
        return Status::Unsupported;
    }

    WEAK_LINK
    auto acpi_init_numa() -> Status {
        return Status::Unsupported;
    }

    static auto x86_init_numa() -> void {
        if (!s_numa_enabled) {
            return;
        }

        if (Status::Ok != acpi_init_numa()) {
            return;
        }

        if (Status::Ok != pci_init_numa()) {
            return;
        }
    }

    /// Before initializing the Physical Memory Manager (PMM), the system should gather 
    /// Non-Uniform Memory Access (NUMA) topology information if the architecture supports it. 
    /// Directly invoking NUMA initialization within `init_platform_early` would introduce 
    /// redundant preprocessor macros to conditionally enable/disable NUMA functionality. 
    /// Instead, encapsulating this logic within a modular initialization hook streamlines 
    /// the codebase by eliminating such macros, improving maintainability and reducing conditional 
    /// compilation overhead.
    GKTL_INIT_HOOK(NumaInit, x86_init_numa, PlatformInitLevel::PrePmm - 1);

} // namespace ours