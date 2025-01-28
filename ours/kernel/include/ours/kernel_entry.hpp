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

#ifndef OURS_KERNEL_ENTRY_HPP
#define OURS_KERNEL_ENTRY_HPP 1

#include <ours/cpu.hpp>
#include <ours/status.hpp>
#include <ours/mem/types.hpp>

#include <ustl/views/span.hpp>

namespace ours {
    struct ArchKernelParam;

    struct KernelParam
    {
        CpuId    cpuid;
        usize    uboot;
        ustl::views::Span<mem::MemRegion>  mem_ranges;
    };

    NO_MANGLE
    auto start_kernel(KernelParam *) -> Status;

    NO_MANGLE
    auto start_nonboot_cpu() -> Status;

} // namespace ours

#endif // #ifndef OURS_KERNEL_ENTRY_HPP