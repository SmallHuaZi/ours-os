// ours OURS_PANIC_HPP
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
#ifndef OURS_PANIC_HPP
#define OURS_PANIC_HPP 1

#include <ours/config.hpp>
#include <ustl/views/string_view.hpp>
#include <ustl/fmt/make_format_args.hpp>

#define OX_PANIC(...)   ours::panic(__VA_ARGS__)

namespace ours {
    NO_RETURN
    auto panic() -> void;

    NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void;

    template <typename... Args>
    NO_RETURN
    auto panic(ustl::views::StringView fmt, Args&&... args) -> void {
        do_panic(fmt, ustl::fmt::make_format_args(args...));
    }

    NO_RETURN FORCE_INLINE 
    static auto unreachable() -> void {
        panic("Unreachable");
    }

} // namespace ours

#endif // #ifndef OURS_PANIC_HPP