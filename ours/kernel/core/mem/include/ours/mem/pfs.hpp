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

#ifndef OURS_MEM_PF_FLAGS_HPP
#define OURS_MEM_PF_FLAGS_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marco_abi.hpp>

#include <ustl/traits/underlying.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    /// Physical frame states
    enum class Pfs: u64 {
        // The following fields indicate the role a PmFrame is playing (occuppied 6 bits)
        Io = BIT(0),
        Lru,
        Mmu,
        Heap,
        RoleMask = BIT_RANGE(0, 6),

        // The following fields point out the type of the current descriptor (occuppied 1 bits).
        Frame = BIT(6),
        Folio,
        TypeMask = BIT_RANGE(6, 7),

        // Zone index occupied 4 bits, mask as BIT_RANGE(7, 11)
        // Node index occupied 9 bits, mask as BIT_RANGE(11, 20)

        // The following fields point out the states of a PmFrame (occupping 16 bits)
        Dirty  = BIT(20),
        Status  = BIT(21),
        Cache  = BIT(22),
        Active = BIT(23),
        Pinned = BIT(24),
        Shared = BIT(25),
    };
    USTL_ENABLE_ENUM_BITS(Pfs);

    CXX11_CONSTEXPR 
    static inline auto zone_index(Pfs flags) -> usize 
    {
        typedef ustl::traits::UnderlyingTypeT<Pfs> Value;
        return (static_cast<Value>(flags) & BIT_RANGE(7, 11)) >> 7;
    };

    CXX11_CONSTEXPR 
    static inline auto node_index(Pfs flags) -> usize 
    {
        typedef ustl::traits::UnderlyingTypeT<Pfs> Value;
        return (static_cast<Value>(flags) & BIT_RANGE(11, 20)) >> 11;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PF_FLAGS_HPP