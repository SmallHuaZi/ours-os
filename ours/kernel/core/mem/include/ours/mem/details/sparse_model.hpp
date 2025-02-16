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

#ifndef OURS_MEM_DETAILS_SPARSE_MODEL_HPP
#define OURS_MEM_DETAILS_SPARSE_MODEL_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>

namespace ours::mem {
    class SparseMemoryModel
    {
        typedef SparseMemoryModel       Self;
    public:
        static auto init() -> Status;

        static auto exist(Pfn frame) -> bool;

        static auto exist(Pfn start, Pfn end) -> bool;

        static auto phys_to_frame(PhysAddr phys_addr) -> PmFrame *
        {  return Self::pfn_to_frame(phys_to_pfn(phys_addr));  }

        static auto frame_to_phys(PmFrame *frame) -> PhysAddr
        {  return pfn_to_phys(Self::frame_to_pfn(frame));  }

        static auto pfn_to_frame(Pfn pfn) -> PmFrame *;

        static auto frame_to_pfn(PmFrame *frame) -> Pfn;

        static auto populate_range(Pfn start_pfn, Pfn end_pfn) -> Status;

        static auto populate_range(Pfn start_pfn, Pfn end_pfn, Altmap *altmap) -> Status;

        static auto depopulate_range(Pfn start_pfn, Pfn end_pfn) -> Status;
    
    private:
        struct Entry
        {
            usize ptr_plus_flags;
        };

        CXX11_CONSTEXPR
        static usize const NR_ROOT_NODES = 32;

        CXX11_CONSTEXPR
        static usize const NR_FRAMES_PER_ROOT = FRAME_SIZE / MAX_FRAME_DESCRIPTOR_NUM_BYTES;

        static Entry ROOT_NODE[NR_ROOT_NODES][NR_FRAMES_PER_ROOT];
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_DETAILS_SPARSE_MODEL_HPP