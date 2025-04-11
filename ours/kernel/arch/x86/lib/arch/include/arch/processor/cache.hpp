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
#ifndef ARCH_PROCESSOR_CACHE_HPP
#define ARCH_PROCESSOR_CACHE_HPP 1

#include <arch/types.hpp>

namespace arch {
    struct X86CpuCacheInfo {
        u32 l1d_size_kb;
        u32 l2d_size_kb;
    };
} // namespace arch

#endif // #ifndef ARCH_PROCESSOR_CACHE_HPP