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
#ifndef OURS_IRQ_MOD_HPP
#define OURS_IRQ_MOD_HPP 1

#include <ours/macro_abi.hpp>
#include <ours/status.hpp>
#include <ours/types.hpp>

#include <ustl/function/fn.hpp>
#include <ustl/result.hpp>

namespace ours::irq {
    enum class IrqReturn {
        None = (0 << 0),
        Handled = (1 << 0),
        WakeThread = (1 << 1),
    };
    using IrqHandler = ustl::function::Fn<IrqReturn(int, void *)>;

    /// TODO(SmallHuaZi) Most of the following types are from Linux, and we rarely
    /// use them. We should remove as many as possible, but we are not yet certain
    /// which flags are necessary.
    enum class IrqFlags {
        None = 0,
        Shared = 1 << 0,
        Exclusive = 1 << 1,
        Disabled = 1 << 2,
        Fast = 1 << 3,
        NoSuspend = 1 << 4,
        NoAutoen = 1 << 5,
        WakeThread = 1 << 6,
        Permanent = BIT(7),
    };

    auto request_irq(HIrqNum irqnum, IrqHandler handler, IrqFlags flags) -> ustl::Result<VIrqNum, Status>;

    auto release_irq(VIrqNum virqnum) -> Status;

    auto handle_irq_generic(HIrqNum irqnum) -> Status;

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_MOD_HPP