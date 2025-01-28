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

#ifndef OURS_MEM_MAPPING_CONTEXT_HPP
#define OURS_MEM_MAPPING_CONTEXT_HPP 1

#include <ours/assert.hpp>

#include <ours/mem/types.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/mmu_flags.hpp>

namespace ours::mem {
    struct MappingContext
    {
        MappingContext(VirtAddr virt_addr, PhysAddr phys_addr, usize n, MmuFlags flags)
            : flags_(flags),
              phys_addr_(phys_addr),
              phys_addr_iter_(phys_addr),
              virt_addr_(virt_addr),
              virt_addr_iter_(virt_addr),
              to_free_()
        {}

        ~MappingContext()
        {  DEBUG_ASSERT(to_free_.empty());  }

        CXX11_CONSTEXPR
        auto virt_addr() const -> VirtAddr
        {  return virt_addr_iter_;  }

        CXX11_CONSTEXPR
        auto phys_addr() const -> PhysAddr
        {  return phys_addr_iter_;  }

        CXX11_CONSTEXPR
        auto flags() const -> MmuFlags
        {  return flags_;  }

        CXX11_CONSTEXPR
        auto size() const -> usize 
        {  return total_size_;  }

        auto skip(usize page_size) -> void;

        auto consume(usize page_size) -> void;

        auto finish() -> void;

        usize total_size_;
        MmuFlags flags_;
        PhysAddr phys_addr_;
        VirtAddr virt_addr_;
        PhysAddr phys_addr_iter_;
        VirtAddr virt_addr_iter_;
        FrameList<> to_free_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MAPPING_CONTEXT_HPP