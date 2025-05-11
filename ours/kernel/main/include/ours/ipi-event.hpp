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
#ifndef OURS_IPI_EVENT_HPP
#define OURS_IPI_EVENT_HPP 1

#include <ours/cpu-mask.hpp>

namespace ours {
    enum class IpiEvent {
        Generic, 
        Resched, 
        Interrupt, 
        Suspend,
    };

    enum class IpiTarget {
        Mask,
        All,
        Self,
    };

    auto mp_init() -> void;

    /// Trigger schedules on other CPUs. Used mostly by inner threading and scheduler logic.
    auto mp_reschedule(CpuMask mask, u32 flags) -> void;

    /// Trigger an interrupt on another cpu without a corresponding reschedule. Used by the 
    /// hypervisor to trigger a vmexit.
    auto mp_interrupt(IpiTarget target, CpuMask mask) -> void;

} // namespace ours

#endif // #ifndef OURS_IPI_EVENT_HPP