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
#ifndef OURS_IRQ_IRQ_OBJECT_HPP
#define OURS_IRQ_IRQ_OBJECT_HPP 1

#include <ours/irq/mod.hpp>
#include <ours/irq/types.hpp>
#include <ours/irq/irq_observer.hpp>
#include <ours/mutex.hpp>

#include <ustl/rc.hpp>
#include <ustl/sync/mutex.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::irq {
    class IrqObject {
        typedef IrqObject Self;
      public:
        static auto create(HIrqNum, VIrqNum, IrqFlags, char const *name) -> Self *;

        auto init(IrqChip *chip) -> Status;

        auto attach(IrqObserver &observer) -> Status;

        auto detach(IrqObserver &observer) -> Status;

        auto notify() -> void;
      private:
        friend IrqHandler;

        IrqObject(HIrqNum, VIrqNum, IrqFlags flags, char const *name);

        GKTL_CANARY(IrqObject, canary_);
        usize nr_intrs_;
        Mutex request_mutex_;
        IrqFlags irqflags_;
        IrqData irqdata_;
        IrqObserverList observers_;
        char const *name_;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_OBJECT_HPP