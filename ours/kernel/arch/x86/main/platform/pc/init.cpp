#include <ours/start.hpp>
#include <ours/mem/init.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void
    {
        // Init APIC
        // Init PMM
    }

    NO_MANGLE INIT_CODE
    auto init_platform() -> void
    {}

} // namespace ours