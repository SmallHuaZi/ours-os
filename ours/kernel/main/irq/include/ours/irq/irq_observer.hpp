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
#ifndef OURS_IRQ_IRQ_OBSERVER_HPP
#define OURS_IRQ_IRQ_OBSERVER_HPP 1

#include <ours/irq/mod.hpp>
#include <ours/irq/types.hpp>

#include <ustl/collections/intrusive/list.hpp>
#include <gktl/canary.hpp>

namespace ours::irq {
    class IrqObserver {
        typedef IrqObserver Self;
      public:
        FORCE_INLINE
        auto on_event(IrqData &data) -> IrqReturn {
            if (handler_) {
                return handler_(virqnum_, data_);
            }
            return IrqReturn::Handled;
        }

      protected:
        IrqFlags flags_;
        VIrqNum virqnum_;
        IrqHandler handler_;
        void *data_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    };
    USTL_DECLARE_LIST(IrqObserver, IrqObserverList, IrqObserver::ManagedOptions);

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_OBSERVER_HPP