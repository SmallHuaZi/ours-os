#include <ours/task/thread.hpp>

#include <ours/task/timer.hpp>
#include <ours/task/auto_preemption_disabler.hpp>
#include <ours/task/mod.hpp>
#include <ours/task/scheduler.hpp>

#include <ours/mem/mod.hpp>
#include <ours/mem/object-cache.hpp>

#include <ustl/traits/char_traits.hpp>
#include <arch/intr_disable_guard.hpp>
#include <arch/halt.hpp>

#include <gktl/init_hook.hpp>


namespace ours::task {
    static mem::ObjectCache   *s_thread_cache;

    static ThreadManagedList s_global_thread_list;

    Thread::Thread(usize priority, char const *name)
        : sched_entity_(),
          name_(name, ustl::traits::CharTraits<char>::length(name)),
          task_state_(),
          thread_state_(ThreadState::Alive),
          waiter_state_(),
          signals_(),
          arch_thread_(),
          user_thread_(),
          aspace_()
    {}

    auto Thread::trampoline() -> void {
        arch::enable_interrupt(); 

        auto const current = Current::get();
        current->task_state_.invoke();

        Current::exit(current->task_state_.retcode());
    }

    auto Thread::spawn(char const *name, usize priority, ThreadStartEntry entry) -> Self * {
        auto self = new (*s_thread_cache, mem::kGafKernel) Self(priority, name);
        ASSERT(self);

        // FIXME(SmallHuaZi): If failed to initialize thread, we should reclaim the memory
        // used by it.

        auto status = self->kernel_stack_.init();
        if (Status::Ok != status) {
            return 0; 
        }
        self->thread_state_ = ThreadState::Initial;
        self->task_state_.init(entry);
        self->arch_thread_.init(VirtAddr(Thread::trampoline));

        MainScheduler::get()->init_thread(*self, BaseProfile(priority));

        s_global_thread_list.push_back(*self);
        return self;
    }

    auto Thread::switch_context(Self *prev, Self *next) -> void {
        Current::set(next);
        ArchThread::switch_context(&prev->arch_thread_, &next->arch_thread_);
    }

    auto Thread::resume() -> Status {
        arch::IntrDisableGuard guard{};
        // Do nothing to a dead thread.
        if (state() == ThreadState::Terminated) {
            return Status::Ok;
        }

        signals_ |= ~Signals::Suspend;

        if (state() == ThreadState::Blocking || state() == ThreadState::Alive) {
            MainScheduler::(*this);
        }

        return Status::Ok;
    }

    auto Thread::suspend() -> Status {
        canary_.verify();

        arch::IntrDisableGuard guard{};
        return Status::Unimplemented;
    }

    auto Thread::detach() -> Status {
        canary_.verify();
        ustl::sync::LockGuard guard(mutex_);

        if (state() != ThreadState::Terminated) {
            flags_ |= ThreadFlags::Detached;
        }

        return Status::Ok;
    }

    auto Thread::bind_user_thread(ustl::Rc<object::ThreadDispatcher> user_thread) -> void {
        canary_.verify();
        DEBUG_ASSERT(state() == ThreadState::Alive);

        ustl::sync::LockGuard guard(mutex_);
        user_thread_ = user_thread;

        flags_ |= ThreadFlags::Detached;
    }

    auto Thread::wakeup_self() -> void {
        ustl::sync::LockGuard guard(mutex_);
        if (thread_state_ != ThreadState::Sleeping) {
            return;
        }
        // Positively sleep is always given successful status.
        waiter_.notify(Status::Ok);
    }

    auto Thread::Current::preempt() -> void {
        MainScheduler::preempt();
    }

    auto Thread::Current::idle() -> void {
        auto current = get();

        // Mark this thread preemptible and wait other one's preemption request.
        current->sched_object().preemption_state().set_pending();

        while (1) {
            arch::suspend();
        }
    }

    auto Thread::Current::sleep_for(Milliseconds ms, bool interruptible) -> Status {
        auto thread = get();

        ustl::sync::LockGuard guard(thread->mutex_);
        Timer timer;
        {
            timer.activate(Deadline(ms), &Thread::wakeup_self, thread);
            thread->set_sleeping();
            thread->waiter_.wait(interruptible, Status::ShouldWait);
        }
        timer.cancel();
        return thread->waiter_.status();
    }

    auto Thread::Current::exit(i32 retcode) -> void {
        auto current = Current::get();

        arch::IntrDisableGuard intr_guard;
        AutoPreemptionDisabler preemption_disabled_guard;
        current->set_terminate();
        log::trace("Thread {} exit", current->name());

        MainScheduler::reschedule(*current);
        unreachable();
    }

    INIT_CODE
    static auto init_thread_cache() -> void {
        s_thread_cache = mem::ObjectCache::create<Thread>("thread-cache", mem::OcFlags::Folio);
        if (!s_thread_cache) {
            panic("Failed to create object cache for Thread");
        }
        log::trace("ThreadCache has been created");
    }
    GKTL_INIT_HOOK(ThreadCacheInit, init_thread_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::task