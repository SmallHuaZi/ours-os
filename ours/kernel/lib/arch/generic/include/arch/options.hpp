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

#ifndef ARCH_OPTIONS_HPP
#define ARCH_OPTIONS_HPP 1

#include <arch/types.hpp>
#include <ustl/util/pack_options.hpp>

namespace arch {
    USTL_TYPE_OPTION(Mutex, Mutex);

    USTL_TYPE_OPTION(PageManager, PageManager);

    USTL_TYPE_OPTION(TlbInvalidator, TlbInvalidator);

    USTL_CONSTANT_OPTION(PagingLevel, usize, PAGING_LEVEL);

} // namespace arch

#endif // #ifndef ARCH_OPTIONS_HPP