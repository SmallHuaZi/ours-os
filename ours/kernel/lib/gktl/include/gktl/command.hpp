// gktl GKTL_COMMAND_HPP
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

namespace gktl {
    struct Command
    {
        typedef auto (*Callback)() -> void;

        char const *name_;
        char const *help_;
        Callback fn_;
    };

} // namespace gktl

#define GKTL_COMMAND(...)

#endif // #ifndef GKTL_COMMAND_HPP