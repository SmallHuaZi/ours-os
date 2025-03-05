#ifndef OURSE_OBJECT_SIGNAL_OBSERVER_H
#define OURSE_OBJECT_SIGNAL_OBSERVER_H 1

#include <ours/types.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::object {
    class KernelObject;

    struct Signal{};
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
        virtual auto on_match(Signal signals) -> void = 0;

        // Called when the registered handle (which refers to a handle to the
        // Dispatcher object) is being destroyed/"closed"/transferred. (The
        // object itself may also be destroyed shortly afterwards.)
        //
        // At the time this is call, it is safe to delete this object: the
        // caller will not interact with it again.
        //
        // WARNING: This is called under Dispatcher's lock
        virtual auto on_cancel(Signal signals) -> void = 0;

        // Determine if this observer matches the given port and key.
        //
        // If true, this object will be removed.
        //
        // WARNING: This is called under Dispatcher's lock.
        virtual auto match_key(void const *port, u64 key) -> bool
        { return false; }

    protected:
        virtual ~SignalObserver() = default;

    private:
        // Dispatcher state, guarded by KernelObject's lock.
        friend KernelObject;
        void const *handle_;
        Signal triggering_signals_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

    USTL_DECLARE_LIST(SignalObserver, SignalObserverList, SignalObserver::ManagedOption);
}

#endif // #ifndef OURSE_OBJECT_SIGNAL_OBSERVER_H