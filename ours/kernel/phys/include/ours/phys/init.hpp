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

#include <omi/header.hpp>
#include <bootmem/bootmem.hpp>

namespace ours::phys {
    struct Aspace;
    struct Handoff;

    extern char const IMAGE_START[] LINK_NAME("__image_start");
    extern char const IMAGE_END[] LINK_NAME("__image_end");

    static auto global_aspace() -> Aspace * {
        extern Aspace   *ASPACE;
        return ASPACE;
    }

    static auto global_bootmem() -> bootmem::IBootMem * {
        extern bootmem::IBootMem   *BOOTMEM;
        return BOOTMEM;
    }

    static auto global_handoff() -> Handoff * {
        extern Handoff  *HANDOFF;
        return HANDOFF;
    }

    auto init_early_console() -> void;

    auto setup_init_data() -> void;

    auto probe_topology() -> void;

    NO_MANGLE NO_RETURN
    auto phys_main(PhysAddr boot_params) -> void;

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_INIT_HPP