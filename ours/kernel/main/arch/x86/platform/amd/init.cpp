#include <ours/init.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto amd_init_arch() -> void
    {}

    NO_MANGLE INIT_CODE
    auto amd_init_platform() -> void
    {}

} // namespace ours