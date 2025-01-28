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
#include <ours/mem/memory_priority.hpp>

namespace ours::mem {
    class SparseMemoryModel
    {
        typedef SparseMemoryModel       Self;
    public:
        static auto init(ustl::views::Span<ZonePriorityInfo> zpis) -> Status;

        static auto phys_to_frame(PhysAddr phys_addr) -> PmFrame *;

        static auto frame_to_phys(PmFrame *frame) -> PhysAddr;

        static auto populate_range(Pfn start_pfn, Pfn end_pfn) -> Status;

        static auto depopulate_range(Pfn start_pfn, Pfn end_pfn) -> Status;
    
    private:
        struct Entry 
        {
            Entry   *subentries;
            PmFrame *frame_array;
        };

        static auto get_entry(PhysAddr pfn) -> Self::Entry *
        {}
    
        CXX11_CONSTEXPR
        static auto const MAX_NR_ROOT_NODES = 32;
        static Entry ROOT_NODE[MAX_NR_ROOT_NODES];
    };

    auto SparseMemoryModel::phys_to_frame(PhysAddr phys_addr) -> PmFrame *
    {
        Pfn pfn = phys_to_pfn(phys_addr);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_DETAILS_SPARSE_MODEL_HPP