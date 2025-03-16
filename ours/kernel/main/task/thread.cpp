#include <ours/task/thread.hpp>

namespace ours::task {
    auto Thread::spawn(char const *name, ThreadStartEntry entry) -> Self *
    {  return 0;  }

    auto Thread::resume() -> void
    {}

    auto Thread::detach() -> void
    {}

    auto Thread::Current::preempt() -> void {
    }

} // namespace ours::task