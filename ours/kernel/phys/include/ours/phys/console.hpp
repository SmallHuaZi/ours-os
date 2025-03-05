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

#ifndef OURS_PHYS_CONSOLE_HPP
#define OURS_PHYS_CONSOLE_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ustl/collections/intrusive/list_hook.hpp>

#define REGISTER_CONSOLE(TYPE, NAME) \
    static TYPE NAME{};\
    LINK_SECTION(".rodata.console")\
    [[gnu::used]] \
    Console *__##NAME = &NAME;

namespace ours::phys {
    struct Console
    {
        Console()
            : hook_()
        {}

        virtual auto write(char const *s, u32 n) -> void = 0;
        virtual auto read(char *s, u32 n) -> u32 = 0;

        virtual auto activate() -> void;
        virtual auto deactivate() -> void;

        ustl::collections::intrusive::ListMemberHook<>  hook_;
        USTL_DECLARE_HOOK_OPTION(Console, hook_, ManagedOptions);
    };
} // namespace ours::phys

#endif // #ifndef OURS_PHYS_CONSOLE_HPP