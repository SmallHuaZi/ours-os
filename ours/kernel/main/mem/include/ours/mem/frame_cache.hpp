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

#ifndef OURS_MEM_FRAME_CACHE_HPP
#define OURS_MEM_FRAME_CACHE_HPP 1

#include <ours/mem/pm_frame.hpp>

#include <ours/cpu_cfg.hpp>
#include <ours/cpu_local.hpp>

#include <ustl/option.hpp>
#include <ustl/sync/mutex.hpp>

#include <ktl/pcp_cache.hpp>

namespace ours::mem {
    struct FrameCacheConfig
        : public ktl::DefaultCacheConfig
    {
        typedef FrameList<>     ObjectContainer;

        struct ObjectAllocator
        {
            auto alloc_object() -> PmFrame *;

            auto free_object(PmFrame *object) -> void;

            usize order;
            usize cpuid;
        };
    };

    struct FrameCache
        : public ktl::PcpCache<PmFrame, FrameCacheConfig>
    {
        typedef ktl::PcpCache<PmFrame, FrameCacheConfig>     Base;
        using Base::Base;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_FRAME_CACHE_HPP