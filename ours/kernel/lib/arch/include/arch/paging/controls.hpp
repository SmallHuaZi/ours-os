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
#ifndef ARCH_PAGING_CONTROLS_HPP
#define ARCH_PAGING_CONTROLS_HPP 1

#include <arch/types.hpp>

namespace arch::paging {
    enum class MapControl {
        SkipIfExisting,
        OverwriteIfExisting,
        ErrorIfExisting,
        TryLargePage = BIT(2),
    };
    USTL_ENABLE_ENUM_BITMASK(MapControl);

    enum class UnMapControl {
        // Allow us to unmap the path to the pte of termnal node but
        // not unmap the terminal node.
        WithoutTerminal,
        OnlyTerminal,
    };
    USTL_ENABLE_ENUM_BITMASK(UnMapControl);

    enum class HarvestControl {
        /// Only updates the frame's age if it is accessed.
        UpdateAge = BIT(0),

        /// If the entry is unaccessed then we zap it. 
        ZapUnaccessed = BIT(1),

        /// If the frame is accessed we remove the accessed flag.
        ResetUnaccessed = BIT(2),
    };
    USTL_ENABLE_ENUM_BITMASK(HarvestControl);

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_CONTROLS_HPP