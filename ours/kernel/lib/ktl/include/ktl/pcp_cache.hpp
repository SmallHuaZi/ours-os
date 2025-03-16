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

#ifndef KTL_PCP_CACHE_HPP
#define KTL_PCP_CACHE_HPP 1

#include <cstdint>
#include <cstddef>

#include <ours/cpu_local.hpp>

#include <ustl/option.hpp>
#include <ustl/collections/array.hpp>
#include <ustl/sync/mutex.hpp>

namespace ktl {
    template <typename Config>
    struct CacheConfigTraits
    {
        constexpr static size_t MaxCpuNum = Config::MaxCpuNum;

        constexpr static size_t DEFAULT_CACHE_CAPACITY = Config::DEFAULT_CACHE_CAPACITY;

        /// Requires CurrentCpu::current_cpu()
        typedef typename Config::CurrentCpu  CurrentCpu;

        /// Requires CurrentCpu::allocate(size_t n) -> CachedObject *
        /// Requires CurrentCpu::deallocate(CachedObject *, size_t n) -> CachedObject *
        typedef typename Config::ObjectAllocator   ObjectAllocator;

        /// Container::insert()
        /// Container::erase()
        /// Container::push_back()
        /// Container::pop_back()
        typedef typename Config::ObjectContainer   ObjectContainer;

        /// Mutex::lock()
        /// Mutex::unlock()
        /// Mutex::try_lock()
        /// Mutex::try_unlock()
        typedef typename Config::Mutex       Mutex;
    };


    template <typename T, typename Config>
    struct SingleCpuCache
    {
        typedef T   CachedObject;
        typedef CacheConfigTraits<Config>                   ConfigTraits;
        typedef typename ConfigTraits::CurrentCpu           CurrentCpu;
        typedef typename ConfigTraits::ObjectAllocator      ObjectAllocator;
        typedef typename ConfigTraits::ObjectContainer      ObjectContainer;
        typedef typename ConfigTraits::Mutex                Mutex;

        SingleCpuCache()
            : mutex_(),
              capacity_(ConfigTraits::DEFAULT_CACHE_CAPACITY),
              container_()
        {}

        auto take_object() -> CachedObject *
        {  return 0;  }

        auto return_object(CachedObject *) -> void
        {}

        Mutex mutex_;
        size_t capacity_;
        ObjectContainer container_;
    };

    template <typename T, typename Config>
    struct PcpCache
    {
        typedef T   CachedObject;
        typedef CacheConfigTraits<Config>                   ConfigTraits;
        typedef typename ConfigTraits::CurrentCpu           CurrentCpu;
        typedef typename ConfigTraits::ObjectAllocator      ObjectAllocator;
        typedef typename ConfigTraits::ObjectContainer      ObjectContainer;
        typedef typename ConfigTraits::Mutex                Mutex;

        typedef SingleCpuCache<CachedObject, Config>  ScpCache;

        auto take_object() -> CachedObject *
        {
            size_t const CpuNum = CurrentCpu::current_cpu();
            if (caches_[CpuNum].has_value()) {
                return caches_[CpuNum]->take_object();
            }
            return 0;
        }

        auto return_object(CachedObject *object) -> void
        {
            size_t const CpuNum = CurrentCpu::current_cpu();
            if (caches_[CpuNum].has_value()) {
                return caches_[CpuNum]->return_object(object);
            }
        }

        auto grow_cache_capacity(int delta) -> void
        {
        }

        ustl::collections::Array<ustl::Option<ScpCache>, ConfigTraits::MaxCpuNum>  caches_;
    };

    struct DefaultCacheConfig
    {
        constexpr static size_t MaxCpuNum = MAX_CPU_NUM;
        constexpr static size_t DefaultCacheCapacity = 32;

        typedef ustl::sync::Mutex   Mutex;

        struct CurrentCpu
        {
            static auto current_cpu() -> size_t
            {  return ours::CpuLocal::cpu_num();  }
        };
    };
} // namespace ktl

#endif // #ifndef KTL_PCP_CACHE_HPP