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
#ifndef OURS_ARCH_CPU_HPP
#define OURS_ARCH_CPU_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>

namespace ours {
    auto arch_current_cpu() -> CpuNum;

    auto arch_activate_cpu(CpuNum) -> Status;

    auto arch_deactivate_cpu(CpuNum) -> Status;

    auto x86_init_percpu(CpuNum cpunum) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_CPU_HPP