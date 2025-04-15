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
#ifndef OURS_MEM_FRAME_QUEUE_HPP
#define OURS_MEM_FRAME_QUEUE_HPP 1

#include <ours/mem/pm_frame.hpp>

#include <ustl/collections/static-vec.hpp>

namespace ours::mem {
    class FrameQueue {
    public:
        CXX11_CONSTEXPR
        static usize const kNumReclaimable = 8;

        auto set_pinned(PmFrame *frame) -> void;
        auto set_anonymous(PmFrame *frame) -> void;
        auto set_reclaimable(PmFrame *frame) -> void;

        auto mark_accessed(PmFrame *frame) -> void;
    private:
        enum QueueType {
            None,
            Anonymous,
            Pinned,
            ReclaimableStart,
            ReclaimableEnd = ReclaimableStart + kNumReclaimable - 1,
            MaxNumQueues,
        };

        using QueueImpl = FrameList<>;
        ustl::collections::StaticVec<QueueImpl, MaxNumQueues> queues;

        ustl::sync::AtomicU32   lrugen;
        ustl::sync::AtomicU32   mrugen;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_FRAME_QUEUES_HPP