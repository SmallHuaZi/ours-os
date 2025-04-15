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

#ifndef OURS_TASK_THREAD_HPP
#define OURS_TASK_THREAD_HPP 1

#include <ours/arch/thread.hpp>
#include <ours/cpu.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/task/types.hpp>

#include <ours/mem/types.hpp>
#include <ours/sched/sched_object.hpp>

/// ours::object::ThreadDispatcher (PS: It is namely user-thread)
#include <ours/object/thread_dispatcher.hpp>

#include <ustl/collections/intrusive/list.hpp>
#include <ustl/rc.hpp>


namespace ours::task {
    class Process;

    struct ThreadState {
        enum class ThreadFlags {
        };

        enum class MemoryAllocationState {
            // All request from a thread which tagged it to allocate memory
            // will not be accepted.
            MemoryAllocationDisabled,
        };
    };

    class Thread
        : public sched::SchedObject {
        typedef Thread Self;

        SCHED_OBJECT_INTERFACE;
      public:
        /// Creates a thread with `name` that will execute `entry` at `priority`. |arg|
        /// will be passed to `entry` when executed, the return value of `entry` will be
        /// passed to Exit().
        /// This call allocates a thread and places it in the global thread list. This
        /// memory will be freed by either Join() or Detach(), one of these must be called.
        /// The thread will not be scheduled until resume() is called.
        static auto spawn(char const *name, ThreadStartEntry entry) -> Self *;

        static auto switch_context(Self &prev, Self &next) -> void;

        Thread();

        template <typename Functor, typename... Args>
        Thread(Functor const &functor, Args &&...args);

        /// The followings is a group of getter.
        auto id() -> int;

        auto state() -> int;

        auto aspace() -> mem::VmAspace *;

        /// Core method family.
        auto detach() -> void;

        auto kill() -> void;

        auto resume() -> void;

        auto suspend() -> void;

        class Current;
      private:
        CpuNum this_cpu_;
        CpuMask cpu_mask_;
        ArchThread arch_thread_;
        ustl::Rc<mem::VmAspace> aspace_;
        ustl::Rc<object::ThreadDispatcher> user_thread_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        ustl::collections::intrusive::ListMemberHook<> proclist_hook_; // Used by process

      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
        USTL_DECLARE_HOOK_OPTION(Self, proclist_hook_, ProcListOptions);
    };
    USTL_DECLARE_LIST(Thread, ThreadProcList, Thread::ProcListOptions);
    USTL_DECLARE_LIST(Thread, ThreadManagedList, Thread::ManagedListOptions);

    class Thread::Current {
        typedef Thread::Current Self;

      public:
        static auto aspace() -> mem::VmAspace * {
            return 0;
        }

        static auto idle() -> void {
        }

        static auto exit(isize retcode) -> void;

        static auto preempt() -> void;
    };

} // namespace ours::task

#endif // #ifndef OURS_TASK_THREAD_HPP