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
#ifndef OURS_PHYS_PRINT_HPP
#define OURS_PHYS_PRINT_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ustl/fmt/make_format_args.hpp>
#include <ustl/views/string_view.hpp>

namespace ours::phys {
    auto vprint(ustl::views::StringView fmt) -> void;
    auto vprint(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void;

    template <typename... Args> 
    auto print(ustl::views::StringView fmt, Args &&...args) -> void
    { vprint(fmt, ustl::fmt::make_format_args(args...)); }

    template <typename... Args> 
    auto println(ustl::views::StringView fmt, Args &&...args) -> void
    { 
        vprint(fmt, ustl::fmt::make_format_args(args...)); 
        vprint("\n");
    }

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_LOG_HPP