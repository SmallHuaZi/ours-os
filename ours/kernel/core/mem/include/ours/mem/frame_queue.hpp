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

#include <ustl/collections/array.hpp>

namespace ours::mem {
    class FrameQueue 
    {
    public:
        CXX11_CONSTEXPR
        static auto const MAX_NR_LRUS = 8;

    private:
        /// Defines a atomic-sized linked list.
        ustl::collections::Array<FrameList<>, MAX_NR_LRUS> frame_lists_;

        ustl::sync::AtomicUsize   lru_gen_;
        ustl::sync::AtomicUsize   mru_gen_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_FRAME_QUEUES_HPP