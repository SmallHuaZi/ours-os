// ustl/sync USTL/SYNC_LOCKGUARD_HPP
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

#ifndef USTL_SYNC_LOCKGUARD_HPP
#define USTL_SYNC_LOCKGUARD_HPP 1

namespace ustl::sync {
    template <typename Lock>
    class LockGuard
    {
    public:
        LockGuard(Lock &lock)
            : lock_(&lock),
              unlocked_(false)
        { lock.lock(); }

        auto unlock() {
            if (!unlocked_) {
                lock_->unlock();
                unlocked_ = true;
            }
        }

        ~LockGuard()
        { unlock(); }
    
    private:
        Lock *lock_;
        bool unlocked_;
    };

    template <typename Lock>
    LockGuard(Lock) -> LockGuard<Lock>;

} // namespace ustl::sync

#endif // #ifndef USTL_SYNC_LOCKGUARD_HPP