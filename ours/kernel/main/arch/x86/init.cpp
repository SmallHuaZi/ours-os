#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/mem/constant.hpp>

namespace ours {
    NO_MANGLE INIT_CODE 
    auto init_arch_early() -> void
    {}

    NO_MANGLE INIT_CODE 
    auto init_arch() -> void
    {}
} // namespace ours