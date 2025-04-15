/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef GKTL_COMMAND_HPP
#define GKTL_COMMAND_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

namespace gktl {
    struct Command {
        typedef auto (*Callback)(int argc, char **argv) -> int;

        char const *cmd_;
        char const *help_;
        Callback fn_;
    };

    auto do_command(int argc, char **argv) -> void;

} // namespace gktl

#define GKTL_COMMAND(NAME, CMD, HANDLER, HELP)               \
namespace commands::NAME {                        \
    LINK_SECTION(".data.rel.ro.commands") FORCE_USED    \
    static ::gktl::Command const s_command_##NAME {             \
        CMD, HELP, HANDLER                              \
    };                                                  \
}

#endif // #ifndef GKTL_COMMAND_HPP