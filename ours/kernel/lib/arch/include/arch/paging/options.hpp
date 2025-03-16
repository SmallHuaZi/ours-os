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
#ifndef ARCH_PAGING_OPTIONS_HPP
#define ARCH_PAGING_OPTIONS_HPP 1

#include <arch/types.hpp>
#include <ustl/util/pack_options.hpp>

namespace arch::paging {
    USTL_TYPE_OPTION(PageSource, PageSource);
    USTL_TYPE_OPTION(PageFlusher, PageFlusher);

    USTL_CONSTANT_OPTION(PagingLevel, usize, PagingLevelV);

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_OPTIONS_HPP