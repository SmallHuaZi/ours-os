#include <ours/task/thread.hpp>

#include <ours/mem/mod.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/sched/mod.hpp>
#include <ours/sched/scheduler.hpp>

#include <ustl/traits/char_traits.hpp>
#include <arch/intr_disable_guard.hpp>

#include <gktl/init_hook.hpp>

namespace ours::task {
    static mem::ObjectCache   *s_thread_cache;

    static ThreadManagedList s_global_thread_list;

    Thread::Thread(usize priority, char const *name)
        : so_(),
          task_state_(),
          name_(name, ustl::traits::CharTraits<char>::length(name)),
          states_(ThreadState::Alive),
          signals_(),
          user_thread_(),
          arch_thread_(),
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
        self->task_state_.init(entry);
        self->arch_thread_.init(VirtAddr(Thread::trampoline));

        sched::MainScheduler::get()->init_thread(*self, sched::BaseProfile(priority));

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
        if (states_ == ThreadState::Terminated) {
            return Status::Ok;
        }

        signals_ |= ~Signals::Suspend;

        if (states_ == ThreadState::Waiting || states_ == ThreadState::Alive) {
            sched::MainScheduler::unblock(*this);
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

        if (states_ != ThreadState::Terminated) {
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

    auto Thread::Current::preempt() -> void {
    }

    auto Thread::Current::idle() -> void {
    }

    auto Thread::Current::exit(i32 retcode) -> void {
        auto current = Current::get();
        sched::MainScheduler::reschedule(*current);

        unreachable();
    }

    INIT_CODE
    static auto init_thread_cache() -> void {
        s_thread_cache = mem::ObjectCache::create<Thread>("vm-mapping-region-cache", mem::OcFlags::Folio);
        if (!s_thread_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("MappingRegionCache has been created");
    }
    GKTL_INIT_HOOK(ThreadCacheInit, init_thread_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::task