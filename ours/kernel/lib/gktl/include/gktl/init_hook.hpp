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

#define GKTL_SYSTEM_INIT_STATE_PHYSBOOT 0

namespace gktl {
    enum class SystemInitState {

    };

    struct InitHook
    {
        typedef auto (*Callback)() -> void;

        Callback    hook_;
        char const *name_;
    };

} // namespace gktl

#define GKTL_INIT_HOOK(NAME, HOOK, LEVEL) \
GKTL_INIT_HOOK_LINK_SECTION(0) \
static const ours::InitHook GKTL_INIT_HOOK_##NAME = { \
    HOOK,   \
    #NAME   \
};

GKTL_INIT_HOOK(CacheInit, [](){}, 0);

#endif // #ifndef HOOK_HOOK_HPP