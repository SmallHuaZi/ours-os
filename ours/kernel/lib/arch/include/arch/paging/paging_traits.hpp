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
#ifndef ARCH_PAGING_PAGING_TRAITS_HPP
#define ARCH_PAGING_PAGING_TRAITS_HPP 1

#include <arch/types.hpp>

#include <ustl/traits/enum.hpp>
#include <ustl/traits/integral.hpp>

namespace arch::paging {
    namespace traits = ustl::traits;

    template <typename Paging>
    struct PagingTraits: public Paging {
        using typename Paging::LevelType;
        static_assert(traits::IsEnumV<LevelType> || traits::IsIntegralV<LevelType>);

        template <LevelType Level>
        using Pte = typename Paging::template Pte<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const NumPtesLog2 = Paging::template NumPtesLog2<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR
        static auto const NumPtes = ULL(1) << NumPtesLog2<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const SizeOfPte = sizeof(typename Pte<Level>::ValueType);

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const SizeOfTable = NumPtes<Level> * SizeOfPte<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const PageSize = Paging::template PageSize<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const NextLevel = Paging::template NextLevel<Level>;
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_PAGING_TRAITS_HPP