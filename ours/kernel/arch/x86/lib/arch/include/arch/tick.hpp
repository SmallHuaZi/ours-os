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
#ifndef ARCH_TICK_HPP
#define ARCH_TICK_HPP 1

#include <x86intrin.h>
#include <arch/types.hpp>

namespace arch {
    struct EarlyTicks {
        typedef EarlyTicks      Self;

        FORCE_INLINE
        static auto get() -> Self {
            return {_rdtsc()};
        }

        FORCE_INLINE
        static auto zero() -> Self {
            return {0};
        }

        u64 tsc;
    };

} // namespace arch

#endif // #ifndef ARCH_TICK_HPP