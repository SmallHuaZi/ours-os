#include <ours/task/thread.hpp>
#include <ours/mem/object-cache.hpp>

namespace ours::task {
    static ustl::Rc<mem::ObjectCache>   s_thread_cache;

    auto Thread::spawn(char const *name, ThreadStartEntry entry) -> Self * {  
        return 0;  
    }

    auto Thread::resume() -> void
    {}

    auto Thread::detach() -> void
    {}

    auto Thread::Current::preempt() -> void {
    }

    auto Thread::Current::idle() -> void {
    }

} // namespace ours::task