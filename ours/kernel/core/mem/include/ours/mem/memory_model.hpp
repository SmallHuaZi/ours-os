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

#ifndef OURS_MEM_MEMORY_MODEL_HPP
#define OURS_MEM_MEMORY_MODEL_HPP 1

/// [Config.MemoryModel]
#ifndef OURS_CONFIG_MEMORY_MODEL
#   define OURS_CONFIG_MEMORY_MODEL Sparse
#endif // #ifndef OURS_CONFIG_MEMORY_MODEL

#include <ours/mem/types.hpp>

namespace ours::mem {
    /// Altmap - Namely alternative map, it is pre-allocated storage for MemoryModel::populate_range 
    /// @base_pfn: base of the entire dev_pagemap mapping
    /// @reserve: pages mapped, but reserved for driver use (relative to @base)
    /// @free: free pages set aside in the mapping for memmap storage
    /// @align: pages reserved to meet allocation alignments
    /// @alloc: track pages consumed, private to vmemmap_populate()
    struct Altmap {
    	Pfn base_pfn;
    	Pfn end_pfn;
    	Pfn reserve;
    	usize free;
    	usize align;
    	usize alloc;
    };
}

/// [Config.64bitsSystem]
#if !defined(OURS_CONFIG_DISABLE_VFLAT_MODEL) && OURS_CONFIG_64BITS_SYSTEM
#   define OURS_CONFIG_ENABLE_VFLAT_MODEL
#endif // #ifdef OURS_CONFIG_64BITS_SYSTEM

#if OURS_CONFIG_MEMORY_MODEL == Sparse
#   include <ours/mem/details/sparse_model.hpp>
#elif OURS_CONFIG_MEMORY_MODEL == Vflat
#   include <ours/mem/details/vflat_model.hpp>
#elif OURS_CONFIG_MEMORY_MODEL == Flat
#   include <ours/mem/details/flat_model.hpp>
#else
#   error "Unsupported physical memory model."
#endif

#include <ours/marco_abi.hpp>

namespace ours::mem {
    /// `MemoryModel` is a high-level abstract and implementation to physical memory layout.
    /// It's responsibility is to pre-allocate `PmFrame` and to provide a group of the convertion
    /// method among `PmFrame`, `Pfn`, `VirtAddr` and `PhysAddr`.
    typedef OURS_MPL_CONTACT(OURS_CONFIG_MEMORY_MODEL, MemoryModel)  MemoryModel;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_MODEL_HPP