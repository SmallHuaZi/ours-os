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
#ifndef OURS_PHYS_BOOT_OPTIONS_HPP
#define OURS_PHYS_BOOT_OPTIONS_HPP 1

#include <ours/types.hpp>

namespace ours::phys {
    struct BootOptions {
        bool aslr_disabled{false};
        char load_target[16]{"kernel_main"};
    };

    inline auto global_boot_options() -> BootOptions * {
        extern BootOptions g_boot_options;
        return &g_boot_options;
    }

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_BOOT_OPTIONS_HPP