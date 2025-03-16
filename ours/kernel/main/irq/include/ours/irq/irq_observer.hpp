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
#include <ustl/collections/intrusive/list_hook.hpp>
#include <gktl/canary.hpp>

namespace ours::irq {
    struct IrqObserver {
        typedef IrqObserver Self;
        IrqFlags flags_;
        VIrqNum virqnum_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_OBSERVER_HPP