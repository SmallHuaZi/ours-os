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

#include <ours/config.hpp>
#include <ours/types.hpp>

#include <ustl/collections/intrusive/list_hook.hpp>

#define REGISTER_CONSOLE(TYPE, NAME, ...) \
    [[gnu::init_priority(1000)]] \
    static TYPE NAME{__VA_ARGS__};

namespace ours::phys {
    struct Console {
        Console(char const *name);
        virtual ~Console() = default;

        virtual auto write(char const *s, u32 n) -> void = 0;
        virtual auto read(char *s, u32 n) -> u32 = 0;

        virtual auto activate() -> void {}
        virtual auto deactivate() -> void {}

        char const *name_;
        ustl::collections::intrusive::ListMemberHook<> hook_;
        USTL_DECLARE_HOOK_OPTION(Console, hook_, ManagedOptions);
    };
} // namespace ours::phys

#endif // #ifndef OURS_PHYS_CONSOLE_HPP