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

#ifndef OURS_ARCH_X86_EPT_PAGING_CONFIG_HPP
#define OURS_ARCH_X86_EPT_PAGING_CONFIG_HPP 1

#include <ours/types.hpp>
#include <ours/marco_abi.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    enum class X86EptFlags: u64 {
    };
    USTL_ENABLE_ENUM_BITS(X86EptFlags);

    struct X86ExtentPagingConfig
    {

        typedef X86EptFlags     ArchMmuFlags;
    };

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_X86_EPT_PAGING_CONFIG_HPP