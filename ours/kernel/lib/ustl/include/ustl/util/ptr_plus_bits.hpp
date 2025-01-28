#ifndef USTL_UTIL_PTR_PLUS_BITS_H
#define USTL_UTIL_PTR_PLUS_BITS_H 1

#include <ustl/config.hpp>

namespace ustl {
    enum class PtrPlusBitsPolicy {
        InLower,
        InUpper,
    };

    template<class T, usize NumBits, PtrPlusBitsPolicy = PtrPlusBitsPolicy::InLower>
    struct PtrPlusBits;

    /// This is the specialization to embed extra bits of information
    /// in a raw pointer. The extra bits are stored in the upper bits of the pointer.
    template<class T, usize  NumBits>
    struct PtrPlusBits<T *, NumBits, PtrPlusBitsPolicy::InUpper>
    {
    };
    
    /// This is the specialization to embed extra bits of information
    /// in a raw pointer. The extra bits are stored in the lower bits of the pointer.
    template<class T, usize  NumBits>
    struct PtrPlusBits<T *, NumBits, PtrPlusBitsPolicy::InLower>
    {
        typedef T *        PtrMut;
    
        USTL_FORCEINLINE USTL_CONSTEXPR
        static auto get_ptr(PtrMut n) USTL_NOEXCEPT -> PtrMut
        {  return PtrMut(usize(n) & usize(~MASK));  }
    
        USTL_FORCEINLINE USTL_CONSTEXPR
        static auto set_ptr(PtrMut &n, PtrMut p) USTL_NOEXCEPT -> void
        {  n = PtrMut(usize(p) | (usize(n) & MASK));  }
    
        USTL_FORCEINLINE USTL_CONSTEXPR
        static auto get_bits(PtrMut n) USTL_NOEXCEPT -> usize
        {  return usize(n) & MASK;  }
    
        USTL_FORCEINLINE 
        static auto set_bits(PtrMut &n, usize c) USTL_NOEXCEPT -> void
        {  n = PtrMut(usize((get_ptr)(n)) | usize(c));  }
    
        USTL_CONSTEXPR
        static const auto MASK = usize((usize(1) << NumBits) - 1);
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_PTR_PLUS_BITS_H