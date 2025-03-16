// arch/paging ARCH/PAGING_PML4_PAGING_TRAITS_HPP
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
#ifndef ARCH_PAGING_L4_PAGING_TRAITS_HPP
#define ARCH_PAGING_L4_PAGING_TRAITS_HPP 1

#include <arch/types.hpp>

namespace arch::paging {
    struct L4Paging {
        typedef usize   LevelType;

        template <LevelType Level>
        struct Pte;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const NumPtesLog2 = 512;

        CXX11_CONSTEXPR
        static usize const PageSizeMap[]
        { KB(4), MB(2), GB(1), GB(512), TB(256) };

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const PageSize = PageSizeMap[Level];
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_L4_PAGING_TRAITS_HPP