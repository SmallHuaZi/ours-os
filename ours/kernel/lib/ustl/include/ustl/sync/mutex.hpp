// ustl/sync USTL/SYNC_MUTEX_HPP
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

#ifndef USTL_SYNC_MUTEX_HPP
#define USTL_SYNC_MUTEX_HPP 1

namespace ustl::sync {
    class Mutex {
    public:
        auto lock() -> void;
        auto unlock() -> void;

        auto try_lock() -> bool;
        auto try_unlock() -> bool;
    };

} // namespace ustl::sync

#endif // #ifndef USTL_SYNC_MUTEX_HPP