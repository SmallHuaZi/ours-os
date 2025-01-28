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

#ifndef ARCH_X86_CACHE_HPP
#define ARCH_X86_CACHE_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

namespace arch {
    struct Cache
    {
        static auto flush(ours::usize phys_addr) -> void
        {
            ours::usize const dirty_line = phys_addr & CACHE_LINE_MASK;
            asm volatile("clflush %0" :: "m"(dirty_line): "memory");
        }

    private:
        static ours::usize const CACHE_LINE_MASK;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_CACHE_HPP