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
#include <ours/cpu-mask.hpp>
#include <ours/ipi-event.hpp>

#include <arch/interrupt.hpp>

namespace ours {
    auto arch_current_cpu() -> CpuNum;

    /// Activate and Deactivate are hot-plug interfaces.
    /// Now we has not supported them.
    auto arch_activate_cpu(CpuNum) -> Status;

    auto arch_deactivate_cpu(CpuNum) -> Status;

    auto arch_send_ipi(IpiTarget target, CpuMask cpus, IpiEvent event) -> Status;

    auto x86_init_percpu(CpuNum cpunum) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_CPU_HPP