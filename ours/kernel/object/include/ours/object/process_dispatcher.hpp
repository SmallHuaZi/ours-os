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
#ifndef OURS_OBJECT_PROCESS_DISPATCHER_HPP
#define OURS_OBJECT_PROCESS_DISPATCHER_HPP 1

#include <ours/task/thread.hpp>
#include <ours/object/dispatcher.hpp>
#include <ours/object/thread_dispatcher.hpp>

#include <ustl/collections/intrusive/list.hpp>

namespace ours::object {
    class VmAspace;

    using ProcessDispatcherBase = SoloDispatcher<ProcessDispatcher, 
                                                 kDefaultRights, 
                                                 Signals::Suspend>;

    class ProcessDispatcher: public ProcessDispatcherBase {
        typedef ProcessDispatcher     Self;
      public:
        auto attach_thread(ThreadDispatcher &thread) -> Status;

        auto detach_thread(ThreadDispatcher &thread) -> Status;
      private:
        Self *parent_;

        /// Unique address space per each process.
        VmAspace *aspace_;

        /// All sub-process forked from this process.
        ustl::collections::intrusive::ListMemberHook<>  sibling_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, sibling_hook_, SiblingListOptions);
        USTL_DECLARE_LIST(Self, SiblingList, SiblingListOptions);
        SiblingList sibling_list_;

        /// All sub-process forked from this process.
        ustl::collections::intrusive::ListMemberHook<>  children_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, sibling_hook_, ChildrenListOptions);
        USTL_DECLARE_LIST(Self, ChildrenList, ChildrenListOptions);
        ChildrenList children_list_;

        /// For some specific case, such as IO-wating, Sched-sleeping.
        ustl::collections::intrusive::ListMemberHook<>  managed_hook_;

        /// All threads belong to this process.
        Mutex thread_list_mutex_;
        ThreadDispatcherList thread_list_;

        /// For all of process.
        ustl::collections::intrusive::ListMemberHook<>  proclist_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, proclist_hook_, ProcListOptions);
        USTL_DECLARE_LIST(Self, Processlist, ProcListOptions);
        static Processlist ALL_PROCESS_LIST;

      public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    };
    USTL_DECLARE_LIST(ProcessDispatcher, ProcessManagedList, ProcessDispatcher::ManagedOptions);

} // namespace ours::object

#endif // OURS_OBJECT_PROCESS_DISPATCHER_HPP
