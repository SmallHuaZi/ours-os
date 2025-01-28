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

/// [Config.64bitsSystem]
#if !defined(OURS_CONFIG_DISABLE_VFLAT_MODEL) && OURS_CONFIG_64BITS_SYSTEM
#   define OURS_CONFIG_ENABLE_VFLAT_MODEL
#endif // #ifdef OURS_CONFIG_64BITS_SYSTEM

#if OURS_CONFIG_MEMORY_MODEL == Sparse
#   include <ours/mem/details/sparse_model.hpp>
#elif OURS_CONFIG_MEMORY_MODEL == Flat
#   include <ours/mem/details/flat_model.hpp>
#else
#   error "Unsupported physical memory model."
#endif

#if OURS_CONFIG_ENABLE_VFLAT_MODEL
#   include <ours/mem/details/vflat_model.hpp>
#endif

#include <ours/marco_abi.hpp>

namespace ours::mem {
    /// `MemoryModel` is a high-level abstract and implementation to physical memory layout.
    /// It's responsibility is to pre-allocate and to manage `PmFrame`.
    typedef OURS_MPL_CONTACT(OURS_CONFIG_MEMORY_MODEL, MemoryModel)  MemoryModel;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_MODEL_HPP