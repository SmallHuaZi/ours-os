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

#ifndef OURS_MEM_PMM_HPP
#define OURS_MEM_PMM_HPP 1

#include <ours/mem/gaf.hpp>
#include <ours/mem/gaf_policy.hpp>
#include <ours/mem/pm_frame.hpp>

#include <ours/status.hpp>

namespace ours::mem {
    [[nodiscard("alloc_frames(): Ignoring the return value will lead to memory leaks.")]] 
    auto alloc_frame(Gaf flags, usize order = 0) -> PmFrame *;

    [[nodiscard("alloc_frames(): Ignoring the return value will lead to memory leaks.")]] 
    auto alloc_frame(GafPolicy policy, Gaf flags, usize order = 0) -> PmFrame *;

    /// Assumptions:
    ///     1) `addr` is not null.
    [[nodiscard("alloc_frames(): Ignoring the return value will lead to memory leaks.")]] 
    auto alloc_frame(Gaf flags, ai_out PhysAddr *addr, usize order = 0) -> PmFrame *;

    [[nodiscard("alloc_frames(): Ignoring the return value will lead to memory leaks.")]] 
    auto alloc_frame_on_node(NodeId id, Gaf flags, usize order = 0) -> PmFrame *;

    auto free_frame(PmFrame *frame) -> Status;

    /// Allocate `n` frames of memory and store them in the provided list.
    auto alloc_frames(Gaf flags, ai_out FrameList<> *list, usize n) -> Status;

    auto free_frames(FrameList<> *list) -> Status;

    auto pin_frame(PmFrame *frame) -> Status;

    auto unpin_frame(PmFrame *frame) -> Status;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PMM_HPP