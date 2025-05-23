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
#ifndef LEGACY_BOOT_HPP
#define LEGACY_BOOT_HPP

#include <ours/phys/init.hpp>

#include <ustl/views/span.hpp>
#include <ustl/views/string_view.hpp>

namespace ours::phys {
    class Aspace;

    struct LegacyBoot {
        /// Any `LegacyBoot` of legacy bootloader should provides a 
        /// specific implementation for this this method.
        auto parse_params(usize params) -> void;

        static auto get() -> LegacyBoot & {
            return g_legacy_boot;
        }

        ustl::views::StringView name_;
        ustl::views::StringView clparams_;
        ustl::views::Span<u8> ramdisk_;
        usize acpi_version; // New version if 1,
        usize acpi_rsdp;

        static LegacyBoot g_legacy_boot;
    };

    auto init_uart_console() -> void;

} // namespace ours::phys

#endif // #ifndef LEGACY_BOOT_HPP