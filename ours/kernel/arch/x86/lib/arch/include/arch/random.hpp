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
#ifndef ARCH_RANDOM_HPP
#define ARCH_RANDOM_HPP 1

#include <arch/types.hpp>

namespace arch {
    CXX11_CONSTEXPR
    static auto const kRdrandRetryLoops = 1000;

    FORCE_INLINE
    static auto rdrand(usize *v) -> bool {
        bool ok;
        auto retry = kRdrandRetryLoops;
        do {
            asm volatile("rdrand %[out]; setc %[ok]"
                : [ok] "=qm" (ok), [out] "=r"(*v));
            if (ok) {
                return true;
            }
        } while (--retry);
        return false;
    }
    
    FORCE_INLINE
    static auto rdseed(usize *v) -> bool {
        bool ok;
        asm volatile("rdseed %[out]; setc %[ok]" 
            : [ok] "=qm" (ok), [out] "=r" (*v));
        return ok;
    }

    FORCE_INLINE
    static auto rand(usize *v) -> bool {
        return rdrand(v);
    }

} // namespace arch

#endif // #ifndef ARCH_RANDOM_HPP