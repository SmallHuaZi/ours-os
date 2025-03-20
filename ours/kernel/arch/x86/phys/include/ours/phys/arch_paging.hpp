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
#ifndef OURS_PHYS_ARCH_PAGING_HPP
#define OURS_PHYS_ARCH_PAGING_HPP 1

#include <arch/paging/x86_pagings.hpp>

namespace ours::phys {
    typedef arch::paging::PagingDispatcher<PAGING_LEVEL>::Type   ArchLowerPaging;
    typedef arch::paging::PagingDispatcher<PAGING_LEVEL>::Type   ArchUpperPaging;

    CXX11_CONSTEXPR
    static PhysAddr const kLowerMappingBase = 0;

    CXX11_CONSTEXPR
    static PhysAddr const kLowerMappingSize = GB(1);

    CXX11_CONSTEXPR
    static PhysAddr const kUpperMappingBase = 0;

    CXX11_CONSTEXPR
    static PhysAddr const kUpperMappingSize = GB(32);

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_ARCH_PAGING_HPP