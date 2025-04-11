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
#ifndef OURS_ARCH_CPU_LOCAL_HPP
#define OURS_ARCH_CPU_LOCAL_HPP 1

#include <arch/x86/msr.hpp>
#include <arch/x86/fsgs.hpp>

namespace ours {
    FORCE_INLINE
    static auto arch_cpu_local_install(usize offset) -> void {
        using namespace arch;
        MsrIo::write(MsrRegAddr::IA32KernelGsBase, offset);
    }

    template <typename Integer>
    FORCE_INLINE
    static auto arch_cpu_local_read(usize offset) -> Integer {
        return arch::read_gs_offset<Integer>(offset);
    }

    template <typename Integer>
    FORCE_INLINE
    static auto arch_cpu_local_write(usize offset, Integer value) -> void {
        arch::write_gs_offset<Integer>(value);
    }

} // namespace ours::arch

#endif // #ifndef OURS_ARCH_CPU_LOCAL_HPP