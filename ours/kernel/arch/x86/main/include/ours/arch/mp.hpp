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
#ifndef OURS_ARCH_MP_HPP
#define OURS_ARCH_MP_HPP 1

#include <ours/cpu-mask.hpp>
#include <ours/ipi-event.hpp>
#include <ours/init.hpp>

#include <ustl/views/span.hpp>

namespace ours {
    auto arch_mp_send_ipi(IpiTarget target, CpuMask mask, IpiEvent event) -> void;

    auto arch_mp_reschedule(CpuMask) -> void;

    auto arch_mp_suspend(CpuMask) -> void;

    auto x86_wakeup_aps(CpuMask) -> void;

namespace task {
    class Thread;
} // namespace task
    NO_MANGLE NO_RETURN INIT_CODE
    auto x86_start_nonboot_cpu(task::Thread *, CpuNum) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_MP_HPP