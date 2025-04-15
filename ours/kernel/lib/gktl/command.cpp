#include <gktl/command.hpp>

namespace gktl {
    extern Command g_commands_start[] LINK_NAME("__commands_start");
    extern Command g_commands_end[] LINK_NAME("__commands_start");

    static auto do_complete() -> void {
    }

    auto do_command(int argc, char **argv) -> void {
    }

} //namespace gktl