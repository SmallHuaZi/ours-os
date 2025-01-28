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

#ifndef ARCH_X86_TLB_HPP
#define ARCH_X86_TLB_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

namespace arch {
    struct Tlb
    {
        static auto flush(ours::usize address) -> void
        {  asm volatile("invlpg %0" :: "m"(address): "memory");  }

        static auto flush_all() -> void
        {  asm volatile("invlpg");  }
    };

} // namespace arch

#endif // #ifndef ARCH_X86_TLB_HPP