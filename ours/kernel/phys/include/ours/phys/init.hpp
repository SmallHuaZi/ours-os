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
#ifndef OURS_PHYS_INIT_HPP
#define OURS_PHYS_INIT_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/assert.hpp>
#include <ours/phys/arch-bootmem.hpp>

namespace ours::phys {
    struct Handoff;
    struct Aspace;

    extern char const kPhysLoadStart[] LINK_NAME("__executable_start");
    extern char const kPhysLoadEnd[] LINK_NAME("__executable_end");

    static auto global_bootmem() -> BootMem * {
        extern BootMem g_bootmem;
        return &g_bootmem;
    }

    extern Handoff *gHandoff;
    static auto global_handoff() -> Handoff * {
        return gHandoff;
    }

    auto arch_init_numa() -> void;

    auto init_early_console() -> void;

    /// This method sets up architecture-specific elements after `BootMem` becomes available, such 
    /// as building page tables and reserving memory for special purposes. It is commonly invoked by 
    /// `init_memory`.
    auto arch_init_memory(Aspace *aspace) -> void;

    /// First, it attempts to parse an OBI item of type OMIT_MEMORY. If no such item is found, 
    /// it will invoke arch_detect_memory to register architecture-specific data into BootMem. 
    /// Once BootMem is initialized, it will set up identity mapping for RAM, provided that |aspace| 
    /// is not NULL.
    auto init_memory(PhysAddr obi, Aspace *aspace) -> void;

    NO_RETURN
    auto obi_main(PhysAddr obi) -> void;

    NO_MANGLE NO_RETURN
    auto phys_main(PhysAddr boot_params) -> void;

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_INIT_HPP