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

#ifndef OURS_CPU_CFG_HPP
#define OURS_CPU_CFG_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#ifndef OURS_CONFIG_MAX_CPU_NUM 
#   define OURS_CONFIG_MAX_CPU_NUM 32
#endif // #ifndef OURS_MAX_CPU_NUM

#ifndef OURS_CONFIG_BOOT_CPU_ID
#   define OURS_CONFIG_BOOT_CPU_ID 0
#endif //#ifndef OURS_BOOT_CPU_ID

namespace ours {
    CXX11_CONSTEXPR
    static u32 const MAX_CPU_NUM = OURS_CONFIG_MAX_CPU_NUM;

    CXX11_CONSTEXPR
    static CpuId const BOOT_CPU_ID{ OURS_CONFIG_BOOT_CPU_ID };

} // namespace ours

#endif // #ifndef OURS_CPU_CFG_HPP