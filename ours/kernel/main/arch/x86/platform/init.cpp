#include <ours/start.hpp>

namespace ours {
    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void
    {}

    NO_MANGLE INIT_CODE
    auto init_platform() -> void
    {}

} // namespace ours