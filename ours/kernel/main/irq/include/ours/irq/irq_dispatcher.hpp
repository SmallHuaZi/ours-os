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
#ifndef OURS_IRQ_IRQ_DISPATCHER_HPP
#define OURS_IRQ_IRQ_DISPATCHER_HPP 1

#include <ours/irq/irq_object.hpp>
#include <ours/irq/irq_chip.hpp>

#include <ustl/views/span.hpp>
#include <ktl/result.hpp>

namespace ours::irq {
    class IrqDispatcher {
      public:
        auto init(usize nr_irqs) -> Status;

        auto request_irq(VIrqNum, IrqHandler, IrqFlags, char const *name) -> Status;

        auto release_irq(VIrqNum) -> Status;

        auto handle_irq(VIrqNum) -> IrqReturn;
      private:
        auto irqnum_to_object(VIrqNum virqnum) -> IrqObject *;

        ustl::views::Span<IrqObject *>  objects_;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_DISPATCHER_HPP