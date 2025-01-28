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

#ifndef OURS_MEM_CFG_HPP
#define OURS_MEM_CFG_HPP 1

#if __has_include(<ours/arch/mem_cfg.hpp>)
#include <ours/arch/cfg_mem.hpp>
#endif

#ifndef OURS_CONFIG_MAX_NR_NODES
#   if OURS_CONFIG_NUMA == 1
#       define OURS_CONFIG_MAX_NR_NODES 32
#   else
#       define OURS_CONFIG_MAX_NR_NODES 1
#   endif
#endif

/// [Node.MaxZoneNum]
#ifndef OURS_CONFIG_MAX_NR_ZONES_PER_NODE
#   define OURS_CONFIG_MAX_NR_ZONES_PER_NODE 4
#elif OURS_CONFIG_MAX_NR_ZONES_PER_NODE > 4
#   error ""
#endif

/// [Frame.Shift]
#ifndef OURS_CONFIG_FRAME_SHIFT
#   define OURS_CONFIG_FRAME_SHIFT   12
#endif

/// [PhysMap.PhysBase]
#ifndef OURS_CONFIG_PHYS_MAP_PHYS_BASE
#   define OURS_CONFIG_PHYS_MAP_PHYS_BASE   0 
#endif

/// [PhysMap.Base]
#ifndef OURS_CONFIG_PHYS_MAP_BASE
#   define OURS_CONFIG_PHYS_MAP_BASE  0x80000000
#endif

/// [PhysMap.Size]
#ifndef OURS_CONFIG_PHYS_MAP_START
#   define OURS_CONFIG_PHYS_MAP_SIZE   0x80000000
#endif

#ifndef OURS_CONFIG_MAX_NR_ZPIS
#   define OURS_CONFIG_MAX_NR_ZPIS 8 
#endif

#endif // #ifndef OURS_MEM_CFG_HPP