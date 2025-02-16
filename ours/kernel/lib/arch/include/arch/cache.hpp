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

#ifndef ARCH_CACHE_HPP
#define ARCH_CACHE_HPP 1

#include <cstdint>

#ifndef ARCH_CONFIG_CACHE_SIZE
#   define ARCH_CONFIG_CACHE_SIZE   64
#endif // #ifndef ARCH_CONFIG_CACHE_SIZE

namespace arch {
    constexpr int32_t const CACHE_SIZE = ARCH_CONFIG_CACHE_SIZE;

} // namespace arch

#endif // #ifndef ARCH_CACHE_HPP