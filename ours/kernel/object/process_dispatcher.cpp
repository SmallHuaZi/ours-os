#include <ours/object/process_dispatcher.hpp>

namespace ours::object {
    auto ProcessDispatcher::attach_thread(ThreadDispatcher &thread) -> Status {
        thread.mark_active();

        thread.activate(false);

        ustl::sync::LockGuard guard(thread_list_mutex_);
        thread_list_.push_back(thread);
        return Status::Ok;
    }

    auto ProcessDispatcher::detach_thread(ThreadDispatcher &thread) -> Status {
        return Status::Unimplemented;
    }

} // namespace ours::object