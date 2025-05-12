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
#ifndef OURS_OBJECT_DISPATCHER_HPP
#define OURS_OBJECT_DISPATCHER_HPP 1

#include <ours/types.hpp>
#include <ours/rights.hpp>
#include <ours/config.hpp>
#include <ours/signals.hpp>
#include <ours/mutex.hpp>
#include <ours/object/types.hpp>
#include <ours/object/observer.hpp>

#include <ustl/rc.hpp>
#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>

#include <ktl/name.hpp>
#include <gktl/canary.hpp>

namespace ours::object {
    class Dispatcher: public ustl::RefCounter<Dispatcher> {
        typedef Dispatcher   Self;
        typedef ustl::RefCounter<Dispatcher>    Base;
      public:
        FORCE_INLINE
        auto koid() const -> KoId {  
            return koid_;  
        }

        auto attach_observer(SignalObserver *, void const *handle, Signals) -> Status;

        auto detach_observer(SignalObserver *, Signals *) -> Status;

        virtual auto on_zero() -> void {};
      protected:
        Dispatcher();
        virtual ~Dispatcher() = default;

        GKTL_CANARY(Dispatcher, canary_);
        KoId koid_;
        Mutex mutex_;
        ustl::sync::AtomicU32  handle_count_;
        ustl::sync::Atomic<Signals>  signals_;

        SignalObserverList observers_;
    };

    template <typename Derived, Rights DefaultRights, Signals ExtraSignals = Signals::None>
    class SoloDispatcher: public Dispatcher {
      public:
        CXX11_CONSTEXPR
        static auto default_right() -> Rights {
            return DefaultRights;
        }
    
      protected:
        GKTL_CANARY(SoloDispatcher, canary_);
    };

} // namespace ours

#endif // #ifndef OURS_OBJECT_DISPATCHER_HPP
