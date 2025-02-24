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

#ifndef GKTL_HOOK_HPP
#define GKTL_HOOK_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#define GKTL_INIT_HOOK_LINK_SECTION(PRIORITY) \
    LINK_SECTION(".kernel.init_hook." #PRIORITY)

namespace gktl {
    enum class InitLevel {
        ArchEarly,
        PlatformEarly,
        VmmInitialized,
        Arch,
        Platform,
    };

    struct InitHook
    {
        typedef auto (*Callback)() -> void;

        Callback    hook_;
        InitLevel   level_;
        char const *name_;
    };

    auto set_init_level(InitLevel level) -> void;

} // namespace gktl

#define GKTL_INIT_HOOK(NAME, HOOK, LEVEL) \
namespace init_hook { \
GKTL_INIT_HOOK_LINK_SECTION(0) \
static const gktl::InitHook GKTL_INIT_HOOK_##NAME##_LEVEL = { \
    HOOK,  \
    LEVEL, \
    NAME   \
}; \
}

#endif // GKTL_HOOK_HPP