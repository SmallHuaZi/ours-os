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
#ifndef ARCH_PROCESSOR_VERSION_HPP
#define ARCH_PROCESSOR_VERSION_HPP 1

#include <arch/types.hpp>

namespace arch {
    /// See Chapter 20 in the Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 1, 
    /// for information on identifying earlier IA-32 processors.
    struct X86CpuVersionInfo {
        u8 stepping_id;
        u8 model;
        u8 family_id;
        u16 display_model;
        u32 display_family_id;
    };
} // namespace arch

#endif // #ifndef ARCH_PROCESSOR_VERSION_HPP