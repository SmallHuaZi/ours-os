// ours/object OURS/OBJECT_OBSERVER_HPP
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
#ifndef OURS_OBJECT_OBSERVER_HPP
#define OURS_OBJECT_OBSERVER_HPP 1

#include <ours/types.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::object {
    class SignalObserver
    {
        typedef SignalObserver      Self;
    public:
        SignalObserver() = default;

        // Called when the set of active signals matches an expected set.
        //
        // At the time this is call, it is safe to delete this object: the
        // caller will not interact with it again.
        //
        // WARNING: This is called under Dispatcher's lock
        virtual auto on_match(SignalMask signals) -> void = 0;

        // Called when the registered handle (which refers to a handle to the
        // Dispatcher object) is being destroyed/"closed"/transferred. (The
        // object itself may also be destroyed shortly afterwards.)
        //
        // At the time this is call, it is safe to delete this object: the
        // caller will not interact with it again.
        //
        // WARNING: This is called under Dispatcher's lock
        virtual auto on_cancel(SignalMask signals) -> void = 0;

        // Determine if this observer matches the given port and key.
        //
        // If true, this object will be removed.
        //
        // WARNING: This is called under Dispatcher's lock.
        virtual auto match_key(void const *port, u64 key) -> bool { 
            return false; 
        }

    protected:
        virtual ~SignalObserver() = default;

    private:
        // Dispatcher state, guarded by KernelObject's lock.
        void const *handle_;
        SignalMask triggering_signals_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };
    USTL_DECLARE_LIST(SignalObserver, SignalObserverList, SignalObserver::ManagedOption);
}

#endif // #ifndef OURS_OBJECT_OBSERVER_HPP