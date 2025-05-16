#include <ours/task/init.hpp>
#include <ours/task/scheduler.hpp>

#include <ours/task/timer-queue.hpp>

namespace ours::task {
    auto init_task_early() -> void {
        // We need this dummy thread as `current` thread to make a feint 
        // that there has been a thread which is running.
        static task::Thread dummy{0, "dummy"};

        new (&dummy) decltype(dummy)(0, "dummy");
        MainScheduler::Current::init_thread(dummy, BaseProfile(0));
        MainScheduler::Current::set_current_thread(&dummy);
        task::Thread::Current::set(&dummy);

        // FIXME(SmallHuaZi): There is no a kernel stack for the first thread.
        // Taking the following way will leak mem::Stack::kDefaultStackSize size of memory.
        dummy.kernel_stack().init();
    }

    auto init_task() -> void {
        MainScheduler::get()->init();

        auto const tq = TimerQueue::get();
        new (tq) TimerQueue();
        log::trace("CPU[0].timer_queue at {:X}", VirtAddr(tq));
    }

} // namespace ours::task