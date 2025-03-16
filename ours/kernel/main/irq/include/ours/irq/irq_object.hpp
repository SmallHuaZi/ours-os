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

#include <ustl/sync/mutex.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::irq {
    /// Run in user address space.
    struct IrqObject {
        typedef IrqObject Self;

        GKTL_CANARY(IrqObject, canary_);
        IrqData data_;
        ustl::sync::Mutex mutex_;

        USTL_DECLARE_LIST(IrqObserver, ActionList, IrqObserver::ManagedOptions);
        ActionList actions_;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_OBJECT_HPP