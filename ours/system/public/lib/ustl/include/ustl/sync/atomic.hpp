// ustl/sync USTL/SYNC_ATOMIC_HPP
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

#ifndef USTL_SYNC_ATOMIC_HPP
#define USTL_SYNC_ATOMIC_HPP 1

#include <atomic>

namespace ustl::sync {
    typedef std::atomic_uint8_t     AtomicU8;
    typedef std::atomic_uint16_t    AtomicU16;
    typedef std::atomic_uint32_t    AtomicU32;
    typedef std::atomic_uint64_t    AtomicU64;
    typedef std::atomic_size_t      AtomicUsize;

    typedef std::atomic_int8_t     AtomicI8;
    typedef std::atomic_int16_t    AtomicI16;
    typedef std::atomic_int32_t    AtomicI32;
    typedef std::atomic_int64_t    AtomicI64;
    typedef std::atomic_ptrdiff_t  AtomicIsize;

    template <typename T>
    using Atomic = std::atomic<T>;

    struct MemoryOrder {
        static auto constexpr Relaxed = std::memory_order::relaxed;
        static auto constexpr Consume = std::memory_order::consume;
        static auto constexpr Acquire = std::memory_order::acquire;
        static auto constexpr Release = std::memory_order::release;
        static auto constexpr AcqRel = std::memory_order::acq_rel; // acquire | release
        static auto constexpr SeqCst = std::memory_order::seq_cst; // acq_rel | 8
    };

} // namespace ustl::sync

#endif // #ifndef USTL_SYNC_ATOMIC_HPP