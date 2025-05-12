#include <ours/object/dispatcher.hpp>
#include <ustl/sync/lockguard.hpp>

namespace ours::object {
    auto Dispatcher::attach_observer(SignalObserver *observer, void const *handle,
                                     Signals signals) -> Status {
        canary_.verify();
        DEBUG_ASSERT(observer);

        ustl::sync::LockGuard guard(mutex_);

        observer->handle_ = handle;
        observer->triggering_signals_ = signals;
        observers_.push_back(*observer);

        return Status::Ok;
    }

    auto Dispatcher::detach_observer(SignalObserver *observer, Signals *signals) -> Status {
        canary_.verify();
        DEBUG_ASSERT(observer);

        ustl::sync::LockGuard guard(mutex_);

        if (signals) {
            *signals = signals_.load(ustl::sync::MemoryOrder::Acquire);
        }

        if (observer->managed_hook_.is_linked()) {
            observers_.erase(observers_.iterator_to(*observer));
        }

        return Status::Ok;
    }
}