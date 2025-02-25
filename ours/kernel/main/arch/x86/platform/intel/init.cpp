#include <ours/init.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto intel_init_arch() -> void
    {}

    NO_MANGLE INIT_CODE
    auto intel_init_platform() -> void
    {}

} // namespace ours
