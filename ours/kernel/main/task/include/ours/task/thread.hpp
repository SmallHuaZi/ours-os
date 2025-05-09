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
#include <ours/cpu-local.hpp>
#include <ours/signals.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/task/types.hpp>

#include <ours/mem/types.hpp>
#include <ours/mem/stack.hpp>
#include <ours/sched/sched_object.hpp>

/// ours::object::ThreadDispatcher (PS: It is so-called user-thread)
#include <ours/object/thread_dispatcher.hpp>

#include <ustl/rc.hpp>
#include <ustl/mem/container_of.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <ktl/name.hpp>
#include <gktl/canary.hpp>

namespace ours::task {
    class Process;

    enum class ThreadStates {
        Alive,
        Ready,
        Running,
        Waiting,
        Terminated,
    };

    enum class ThreadFlags {
        Preemptible = BIT(0),
        SignalPending = BIT(1),

        // All request from a thread which tagged it to allocate memory
        // will not be accepted.
        MemoryAllocationDisabled = BIT(2),
    };

    struct ThreadState {
    };

    class Thread  {
        typedef Thread Self;
      public:
        FORCE_INLINE
        static auto of(sched::SchedObject *so) -> Self * {
            return ustl::mem::container_of(so, &Self::so_);
        }

        FORCE_INLINE
        static auto of(ArchThread *arch) -> Self * {
            return ustl::mem::container_of(arch, &Self::arch_thread_);
        }

        FORCE_INLINE
        static auto current_thread() -> Self * {
            return CpuLocal::read(s_current_thread_);
        }

        /// Creates a thread with `name` that will execute `entry` at `priority`. |arg|
        /// will be passed to `entry` when executed, the return value of `entry` will be
        /// passed to Exit().
        /// This call allocates a thread and places it in the global thread list. This
        /// memory will be freed by either Join() or Detach(), one of these must be called.
        /// The thread will not be scheduled until resume() is called.
        static auto spawn(ThreadStartEntry entry, void *args, char const *name, usize priority) -> Self *;

        static auto switch_context(Self *prev, Self *next) -> void;

        Thread(usize priority, ThreadStartEntry entry, void *args, char const *name);

        // template <typename Functor, typename... Args>
        // Thread(Functor const &functor, Args &&...args);

        /// The followings is a group of getter.
        auto id() -> int;

        auto state() -> int;

        auto aspace() -> mem::VmAspace * {
            return aspace_.as_ptr_mut();
        }

        auto sched_object() -> sched::SchedObject & {
            return so_;
        }

        /// Core method family.
        auto detach() -> Status;

        auto kill() -> void;

        auto resume() -> Status;

        auto suspend() -> Status;

        auto set_cpu_affinity(CpuMask const &) -> void;

        auto kernel_stack() -> mem::Stack & {
            return kernel_stack_;
        }

        class Current;
      private:
        friend ArchThread;

        FORCE_INLINE
        static auto set_current_thread(Self *curr) -> void {
            CpuLocal::write(s_current_thread_, curr);
        }

        GKTL_CANARY(Thread, canary_);
        CpuNum recent_cpu_;
        CpuMask cpumask_;
        ArchThread arch_thread_;

        // Null if kernel thread.
        ustl::Weak<mem::VmAspace> aspace_;
        ustl::Rc<object::ThreadDispatcher> user_thread_;
        mem::Stack kernel_stack_;

        Signals signals_;
        ThreadStates states_;

        void *args_;
        i32 retcode_;
        ThreadStartEntry entry_point_;
        ktl::Name<32> name_;

        sched::SchedObject so_;

        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        ustl::collections::intrusive::ListMemberHook<> proclist_hook_; // Used by process

        /// Cpu local variable, do not use directly instead of Thread::current_thread().
        CPU_LOCAL
        static inline Self *s_current_thread_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
        USTL_DECLARE_HOOK_OPTION(Self, proclist_hook_, ProcListOptions);
    };
    USTL_DECLARE_LIST(Thread, ThreadProcList, Thread::ProcListOptions);
    USTL_DECLARE_LIST(Thread, ThreadManagedList, Thread::ManagedListOptions);

    class Thread::Current {
      public:
        static auto aspace() -> mem::VmAspace * {
            return 0;
        }

        FORCE_INLINE
        static auto sched_object() -> sched::SchedObject & {
            return current_thread()->sched_object();
        }

        FORCE_INLINE
        static auto preemption_state() -> sched::PreemptionState & {
            return current_thread()->sched_object().preemption_state();
        }

        static auto idle() -> void;

        static auto exit(isize retcode) -> void;

        static auto preempt() -> void;
    };

} // namespace ours::task

#endif // #ifndef OURS_TASK_THREAD_HPP