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

#include <arch/paging/paging_dispatcher.hpp>

#include <ustl/array.hpp>
#include <ustl/traits/enum.hpp>
#include <ustl/traits/integral.hpp>

namespace arch::paging {
    namespace traits = ustl::traits;

    template <typename Paging>
    struct PagingTraits: public Paging {
        typedef typename Paging::LevelType  LevelType;
        static_assert(traits::IsEnumV<LevelType> || traits::IsIntegralV<LevelType>);

        using Paging::kPagingLevel;
        using Paging::kTableAlignmentLog2;
        using Paging::kMaxPhysAddrSize;
        using Paging::kVirtAddrExt;

        CXX11_CONSTEXPR 
        static auto const kTableAlignment = BIT(kTableAlignmentLog2);

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kNumPtesLog2 = Paging::template kNumPtesLog2<Level>;

        template <LevelType Level>
        CXX11_CONSTEXPR
        static auto const kNumPtes = BIT(kNumPtesLog2<Level>);

        template <LevelType Level>
        using Pte = typename Paging::template Pte<Level>;

        template <LevelType Level>
        using PteVal = typename Pte<Level>::ValueType;

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kSizeOfPte = sizeof(PteVal<Level>);

        template <LevelType Level>
        CXX11_CONSTEXPR 
        static auto const kSizeOfTable = kNumPtes<Level> * kSizeOfPte<Level>;

        static auto const kFinalLevel = Paging::kAllLevels[0];

        /// The methods below provide the capability to dynamically dispatch 
        /// handling logic across different levels of the page table.
        FORCE_INLINE CXX11_CONSTEXPR
        static auto next_level(LevelType level) -> LevelType {
            return Paging::kAllLevels[usize(level) - 1];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto virt_to_index(LevelType level, VirtAddr addr) -> usize {
            return Paging::virt_to_index(level, addr);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto page_size(LevelType level) -> usize {
            return Paging::kPageSizeMap[usize(level)];
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto max_entries(LevelType level) -> usize {
            return Paging::max_entries(level);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        static auto level_can_be_terminal(LevelType level) -> bool {
            return Paging::level_can_be_terminal(level);
        }
    };

    template <auto PagingLevel, typename... Options>
    struct MakePagingTraits {
        typedef typename PagingDispatcher<PagingLevel, Options...>::Type Paging;
        typedef PagingTraits<Paging>    Type;
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_PAGING_TRAITS_HPP