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

#ifndef OURS_PHYS_HANDOFF_HPP
#define OURS_PHYS_HANDOFF_HPP 1

#include <bootmem/bootmem.hpp>

#include <ustl/views/span.hpp>

namespace ours::phys {
    struct MemoryHandoff
    {
        PhysAddr bootmem_base;
        PhysAddr bootmem_size;
        bootmem::IBootMem *bootmem ai_virt_addr;
    };

    struct Handoff
    {
        MemoryHandoff mem;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_HANDOFF_HPP