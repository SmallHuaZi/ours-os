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

namespace ours {
    auto arch_mp_send_ipi(IpiTarget target, CpuMask mask, IpiEvent event) -> void;

    auto arch_mp_reschedule(CpuMask) -> void;

    auto arch_mp_suspend(CpuMask) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_MP_HPP