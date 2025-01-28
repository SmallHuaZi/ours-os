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

#ifndef COMMAND_COMMAND_HPP
#define COMMAND_COMMAND_HPP

#include <ours/config.hpp>

namespace cmd {
    struct Command
    {
        typedef auto (*Callback)() -> void;

        char const *name_;
        char const *help_;
        Callback fn_;
    };

} // namespace cmd

#endif // COMMAND_COMMAND_HPP