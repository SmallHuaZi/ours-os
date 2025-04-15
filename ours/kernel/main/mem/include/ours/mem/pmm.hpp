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
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/node-mask.hpp>
#include <ours/mem/node-states.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/status.hpp>

#include <ustl/bit.hpp>

namespace ours::mem {
    FORCE_INLINE CXX11_CONSTEXPR
    static auto pmm_enabled() -> bool {
        extern bool g_pmm_enabled;
        return g_pmm_enabled;
    }

    auto bind_cpu_to_node(CpuNum cpunum, NodeId nid) -> void;

    auto cpu_to_node(CpuNum cpunum) -> NodeId;

    extern PhysAddr g_max_phys_addr;
    extern PhysAddr g_min_phys_addr;

    FORCE_INLINE CXX11_CONSTEXPR
    static auto max_pfn() -> Pfn {
        return phys_to_pfn(g_max_phys_addr);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto min_pfn() -> Pfn {
        return phys_to_pfn(g_min_phys_addr);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto size_to_order(usize nr_frames) -> usize {
        return ustl::bit_floor(nr_frames);
    }

    /// Prefer to allocate a frame on the local node.
    OM_API auto alloc_frame(Gaf, usize, NodeMask const & = node_online_mask()) -> PmFrame *;
    OM_API auto alloc_frames(Gaf, ai_out FrameList<> *, usize, NodeMask const & = node_online_mask()) -> Status;

    OM_API FORCE_INLINE
    static auto alloc_frame(Gaf gaf, ai_out PhysAddr *addr, usize order = 0) -> PmFrame * {
        if (auto frame = alloc_frame(gaf, order)) {
            if (addr) {
                *addr = frame_to_virt(frame);
            }
        }

        return nullptr;
    }

    OM_API FORCE_INLINE
    static auto alloc_frame_n(Gaf gaf, usize nr_frames, NodeMask const &nodes = node_online_mask()) -> PmFrame * {
        return alloc_frame(gaf, size_to_order(nr_frames), nodes);
    }

    /// Allocate the a frame on a specified node.
    OM_API auto alloc_frames(NodeId, Gaf, ai_out FrameList<> *, usize, NodeMask const & = node_online_mask()) -> Status;
    OM_API auto alloc_frame(NodeId, Gaf, usize, NodeMask const & = node_online_mask()) -> PmFrame *;

    OM_API FORCE_INLINE
    static auto alloc_frame(NodeId nid, Gaf gaf, ai_out PhysAddr *addr, usize order = 0) -> PmFrame * {
        if (auto frame = alloc_frame(nid, gaf, order)) {
            if (addr) {
                *addr = frame_to_virt(frame);
            }
        }

        return nullptr;
    }

    OM_API FORCE_INLINE
    static auto alloc_frame_n(NodeId nid, Gaf gaf, usize nr_frames, NodeMask const &nodes = node_online_mask()) -> PmFrame * {
        return alloc_frame(nid, gaf, size_to_order(nr_frames), nodes);
    }

    template <typename T>
    [[nodiscard("get_frame(): Ignoring the return value will lead to memory leaks.")]] 
    auto get_frame(Gaf gaf, usize order = 0) -> T * {
        if (auto frame = alloc_frame(gaf, order)) {
            return PhysMap::phys_to_virt<T>(frame_to_phys(frame));
        }

        return nullptr;
    }

    template <typename T>
    [[nodiscard("get_frame(): Ignoring the return value will lead to memory leaks.")]] 
    auto get_frame(NodeId nid, Gaf gaf, usize order = 0) -> T * {
        if (auto frame = alloc_frame(nid, gaf, order)) {
            return PhysMap::phys_to_virt<T>(frame_to_phys(frame));
        }

        return nullptr;
    }

    auto free_frame(PmFrame *frame, usize order = 0) -> void;

    auto free_frame(VirtAddr virt_addr, usize order = 0) -> void;

    /// Allocate `n` frames of memory and store them in the given list.
    auto alloc_frames(Gaf gaf, ai_out FrameList<> *list, usize n) -> Status;

    auto free_frames(FrameList<> *list) -> void;

    auto pin_frame(PmFrame *frame) -> Status;

    auto unpin_frame(PmFrame *frame) -> Status;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PMM_HPP