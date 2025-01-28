#ifndef OURS_TASK_PROCESS_HPP
#define OURS_TASK_PROCESS_HPP 1

#include <ours/task/thread.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::task {
    class VmAspace;

    class Process {
        typedef Process     Self;

    public:

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
        ThreadManagedList thread_list_;

        /// For all of process.
        ustl::collections::intrusive::ListMemberHook<>  proclist_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, proclist_hook_, ProcListOptions);
        USTL_DECLARE_LIST(Self, Processlist, ProcListOptions);
        static Processlist ALL_PROCESS_LIST;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOptions);
    };

    USTL_DECLARE_LIST(Process, ProcessManagedList, Process::ManagedOptions);

} // namespace ours::task

#endif // #ifndef OURS_TASK_PROCESS_HPP 