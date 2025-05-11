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
#include <ustl/function/bind.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <ktl/name.hpp>
#include <gktl/canary.hpp>

namespace ours::task {
    class Process;

    enum class ThreadState {
        Alive,
        Ready,
        Running,
        Waiting,
        Terminated,
    };

    enum class ThreadFlags {
        Preemptible = BIT(0),
        SignalPending = BIT(1),
        Detached = BIT(2),

        // All request from a thread which tagged it to allocate memory
        // will not be accepted.
        MemoryAllocationDisabled = BIT(3),
    };
    USTL_ENABLE_ENUM_BITMASK(ThreadFlags)

    class TaskState {
        typedef TaskState    Self;
      public:
        FORCE_INLINE
        auto init(ThreadStartEntry entry) -> void {
            entry_point_ = entry; 
            retcode_ = 0;
        }

        FORCE_INLINE
        auto invoke() -> void {
            DEBUG_ASSERT(entry_point_);
            retcode_ = entry_point_();
        }

        FORCE_INLINE
        auto retcode() -> i32 {
            return retcode_;
        }

      private:
        i32 retcode_;
        ThreadStartEntry entry_point_;
    };

    class Thread  {
        typedef Thread Self;
      public:
        class Current;

        FORCE_INLINE
        static auto of(sched::SchedObject *so) -> Self * {
            return ustl::mem::container_of(so, &Self::so_);
        }

        FORCE_INLINE
        static auto of(ArchThread *arch) -> Self * {
            return ustl::mem::container_of(arch, &Self::arch_thread_);
        }

        /// Creates a thread with `name` that will execute `entry` at `priority`. |arg|
        /// will be passed to `entry` when executed, the return value of `entry` will be
        /// passed to Exit().
        /// This call allocates a thread and places it in the global thread list. This
        /// memory will be freed by either Join() or Detach(), one of these must be called.
        /// The thread will not be scheduled until resume() is called.
        static auto spawn(char const *name, usize priority, ThreadStartEntry entry) -> Self *;

        template <typename F, typename... Args>
        static auto spawn(char const *name, usize priority, F entry, Args&&... args) -> Self *;

        static auto switch_context(Self *prev, Self *next) -> void;

        Thread(usize priority, char const *name);

        // template <typename Functor, typename... Args>
        // Thread(Functor const &functor, Args &&...args);

        /// The followings is a group of getter.
        auto id() -> int;

        FORCE_INLINE
        auto state() -> ThreadState {
            return states_;
        }

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

        auto bind_user_thread(ustl::Rc<object::ThreadDispatcher> user_thread) -> void;

        FORCE_INLINE
        auto kernel_stack() -> mem::Stack & {
            return kernel_stack_;
        }

        FORCE_INLINE
        auto recent_cpu() const -> CpuNum {
            return so_.recent_cpu();
        }

        FORCE_INLINE
        auto set_ready() -> Self & {
            states_ = ThreadState::Ready;
            return *this;
        }

        FORCE_INLINE
        auto set_running() -> Self & {
            states_ = ThreadState::Running;
            return *this;
        }

        FORCE_INLINE
        auto set_blocking() -> Self & {
            states_ = ThreadState::Waiting;
            return *this;
        }
      private:
        friend ArchThread;

        NO_RETURN
        static auto trampoline() -> void;

        GKTL_CANARY(Thread, canary_);
        ArchThread arch_thread_;
        ktl::Name<32> name_;

        // Null if kernel thread.
        ustl::Weak<mem::VmAspace> aspace_;
        ustl::Rc<object::ThreadDispatcher> user_thread_;
        mem::Stack kernel_stack_;

        Mutex mutex_;
        Signals signals_;
        ThreadFlags flags_;
        ThreadState states_;
        TaskState task_state_;
        sched::SchedObject so_;

        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        ustl::collections::intrusive::ListMemberHook<> proclist_hook_; // Used by process
      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
        USTL_DECLARE_HOOK_OPTION(Self, proclist_hook_, ProcListOptions);
    };
    USTL_DECLARE_LIST(Thread, ThreadProcList, Thread::ProcListOptions);
    USTL_DECLARE_LIST(Thread, ThreadManagedList, Thread::ManagedListOptions);

    class Thread::Current {
      public:
        FORCE_INLINE
        static auto aspace() -> mem::VmAspace * {
            return 0;
        }

        FORCE_INLINE
        static auto get() -> Self * {
            return CpuLocal::read(s_current_thread_);
        }

        FORCE_INLINE
        static auto set(Self *curr) -> Self * {
            auto const prev = get();
            CpuLocal::write(s_current_thread_, curr);
            return prev; 
        }

        FORCE_INLINE
        static auto sched_object() -> sched::SchedObject & {
            return get()->sched_object();
        }

        FORCE_INLINE
        static auto preemption_state() -> sched::PreemptionState & {
            return get()->sched_object().preemption_state();
        }

        static auto idle() -> void;

        NO_RETURN
        static auto exit(i32 retcode) -> void;

        static auto preempt() -> void;

        /// Cpu local variable, do not use directly instead of Thread::current_thread().
        CPU_LOCAL
        static inline Self *s_current_thread_;
    };

    template <typename F, typename... Args>
    FORCE_INLINE
    auto Thread::spawn(const char *name, usize priority, F entry, Args&&... args) -> Self * {
        auto entry_point = ustl::function::bind(entry, ustl::forward<Args>(args)...);
        return spawn(name, priority, ThreadStartEntry(entry_point));
    }

} // namespace ours::task

#endif // #ifndef OURS_TASK_THREAD_HPP