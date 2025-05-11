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
#ifndef OURS_OBJECT_THREAD_DISPATCHER_HPP
#define OURS_OBJECT_THREAD_DISPATCHER_HPP 1

#include <ours/object/dispatcher.hpp>
#include <ours/object/handle.hpp>
#include <ours/task/types.hpp>

#include <ktl/name.hpp>
#include <gktl/canary.hpp>
#include <ustl/option.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::object {
    using ThreadDispatcherBase = SoloDispatcher<ThreadDispatcher, 
                                                kDefaultRights, 
                                                Signals::Suspend>;

    class ThreadDispatcher: public ThreadDispatcherBase {
        typedef ThreadDispatcher    Self;
      public:
        enum class Flags {
            Active = BIT(0),
        };
        USTL_ENABLE_INNER_ENUM_BITMASK(Flags);

        struct TaskState {
            usize ip = 0;
            usize sp = 0;
            usize arg1 = 0;
            usize arg2 = 0;
        };

        static auto spawn(ustl::Rc<ProcessDispatcher> process, char const *name,
                          KernelHandle<Self> *out) -> Status;
        
        auto start(TaskState const &state) -> Status;

        auto resume() -> Status;

        auto activate(bool suspend) -> Status;

        FORCE_INLINE
        auto mark_active() -> void {
            flags_ |= Flags::Active;
        }
      private:
        ThreadDispatcher(ustl::Rc<ProcessDispatcher> process, char const *name);

        static auto trampoline(Self *) -> i32;

        GKTL_CANARY(ThreadDispatcher, canary_);
        Flags flags_;
        ustl::Rc<ProcessDispatcher> process_;
        task::Thread *kernel_thread_;
        ustl::Option<TaskState> user_entry_;

        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    }; // class ThreadDispatcher

    using ThreadDispatcherList = ustl::collections::intrusive::List<ThreadDispatcher, ThreadDispatcher::ManagedOptions>;

} // namespace ours::object

#endif // #ifndef OURS_OBJECT_THREAD_DISPATCHER_HPP