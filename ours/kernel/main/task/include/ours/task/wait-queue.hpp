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
#ifndef OURS_TASK_WAIT_QUEUE_HPP
#define OURS_TASK_WAIT_QUEUE_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>
#include <ustl/collections/intrusive/set.hpp>

namespace ours::task {
    class Waiter {
        typedef Waiter Self;
      public:
        auto wait(bool interruptible, Status status) -> void;

        auto notify(Status status) -> void;

        FORCE_INLINE
        auto status() const -> Status {
            return status_;
        }
      private:
        Status status_;
        ustl::collections::intrusive::SetMemberHook<> managed_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

    class WaitQueue {
      public:

      private:
        USTL_DECLARE_MULTISET(Waiter, WaiterSet, Waiter::ManagedOption);
        WaiterSet waiters_;
    };

} // namespace ours::task

#endif // #ifndef OURS_TASK_WAIT_QUEUE_HPP