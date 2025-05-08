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
#include <ours/config.hpp>

#include <ustl/function/fn.hpp>
#include <ustl/result.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::irq {
    CXX11_CONSTEXPR
    static auto const kInvalidVIrqNum = VIrqNum(-1);

    enum class IrqReturn {
        None = (0 << 0),
        Handled = (1 << 0),
        WakeThread = (1 << 1),
    };
    using IrqHandler = ustl::function::Fn<IrqReturn(VIrqNum, void *)>;

    enum class IrqFlags {
        // For semantics, no any real effects
        None = 0,

        TriggerRising  = BIT(0),
        TriggerFalling = BIT(1),
        TriggerHigh = BIT(2),
        TriggerLow	= BIT(3),
        TriggerMask = TriggerRising | TriggerFalling | TriggerHigh | TriggerLow,

        Shared = BIT(4),
        Disabled = BIT(5),
        Fast = BIT(6),
        NoSuspend = BIT(7),
        NoAutoen = BIT(8),
        WakeThread = BIT(9),
        Permanent = BIT(10),
    };
    USTL_ENABLE_ENUM_BITMASK(IrqFlags);

    auto init_early(usize num_irqs) -> void;

    auto request_irq(VIrqNum, IrqHandler, IrqFlags, char const *) -> Status;

    auto release_irq(VIrqNum) -> Status;

    auto handle_irq_generic(VIrqNum irqnum) -> IrqReturn;

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_MOD_HPP