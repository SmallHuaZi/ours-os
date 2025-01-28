#ifndef USTL_UTIL_ALIGN_H
#define USTL_UTIL_ALIGN_H 1

#include <ustl/config.hpp>

namespace ustl::mem {
    USTL_FORCEINLINE USTL_CONSTEXPR  
    auto align_down(usize n, usize a) USTL_NOEXCEPT -> usize 
    { return n & ~(a - 1); }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto align_up(usize n, usize a) USTL_NOEXCEPT -> usize
    { return (n + a - 1) & ~(a - 1); }

} // namespace ustl::mem

#endif // USTL_UTIL_ALIGN_H