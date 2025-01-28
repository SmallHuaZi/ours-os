#ifndef USTL_SYNC_MUTEX_H
#define USTL_SYNC_MUTEX_H 1

namespace ustl::sync {
    class Mutex
    {
    public:
        auto lock() -> void;
        auto unlock() -> void;
        auto try_lock() -> void;
        auto try_unlock() -> void;
    };

} // namespace ustl::sync

#endif // #ifndef USTL_SYNC_MUTEX_H