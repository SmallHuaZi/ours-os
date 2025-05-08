#include <ours/task/thread.hpp>

#include <ours/mem/mod.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/sched/mod.hpp>

#include <ustl/traits/char_traits.hpp>

#include <arch/intr_disable_guard.hpp>

namespace ours::task {
    static mem::ObjectCache   *s_thread_cache;

    static ThreadManagedList s_global_thread_list;

    Thread::Thread(usize priority, ThreadStartEntry entry, void *args, char const *name)
        : so_(),
          entry_point_(entry), 
          args_(args), 
          name_(name, ustl::traits::CharTraits<char>::length(name)),
          states_(ThreadStates::Alive),
          retcode_(0),
          signals_(),
          user_thread_(),
          arch_thread_()
    {}

    auto Thread::switch_context(Self *prev, Self *next) -> void {
        set_current_thread(next);

        ArchThread::switch_context(&prev->arch_thread_, &next->arch_thread_);
    }

    auto Thread::spawn(ThreadStartEntry entry, void *args, char const *name, usize priority) -> Self * {
        auto self = new (*s_thread_cache, mem::kGafKernel) Self(priority, entry, args, name);
        ASSERT(self);

        s_global_thread_list.push_back(*self);
        return self;
    }

    auto Thread::resume() -> Status {
        arch::IntrDisableGuard guard{};
        // Do nothing to a dead thread.
        if (states_ == ThreadStates::Terminated) {
            return Status::Ok;
        }

        signals_ |= ~Signals::Suspend;

        if (states_ == ThreadStates::Waiting || states_ == ThreadStates::Alive) {
            sched::activate(*this);
            states_ = ThreadStates::Running;
        }

        return Status::Ok;
    }

    auto Thread::suspend() -> Status {
        arch::IntrDisableGuard guard{};
        return Status::Unimplemented;
    }

    auto Thread::detach() -> Status {
        arch::IntrDisableGuard guard{};
        return Status::Unimplemented;
    }

    auto Thread::Current::preempt() -> void {
    }

    auto Thread::Current::idle() -> void {
    }

} // namespace ours::task