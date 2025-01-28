#ifndef USTL_UTIL_BITOPS_H
#define USTL_UTIL_BITOPS_H 1

#include <ustl/config.hpp>

namespace ustl {
namespace details {

    USTL_CONSTEXPR static usize const WORD_MAX = usize(-1);
    USTL_CONSTEXPR static usize const WORD_BITS = sizeof(usize) << 3;

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto which_bit(usize pos) -> usize
    {
        return pos & 7;
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto which_byte(usize pos) -> usize
    {
        return pos >> 3;
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto which_word(usize pos) -> usize
    {
        return pos / sizeof(usize);
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto make_bit(usize pos) -> usize
    {
        return usize(1) << which_bit(pos);
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto mask_bits(usize pos) -> usize
    {
        return ~make_bit(pos);
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto mask_high_bits(usize pos) -> usize
    {
        return WORD_MAX >> pos << pos; 
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto mask_low_bits(usize pos) -> usize
    {
        return ~mask_bits(pos) - 1;
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_test(usize const *map, usize pos) -> bool
    {
        usize i = which_word(pos);
        return (map[i] & make_bit(pos)) != 0;
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_flip(usize *map, usize pos) -> void
    {
        usize i = which_word(pos);
        map[i] = mask_bits(pos) ^ ~map[i];
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_clr(usize *map, usize pos) -> void
    {
        usize i = which_word(pos);
        map[i] &= mask_bits(pos);
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_set(usize *map, usize pos) -> void
    {
        usize i = which_bit(pos);
        usize j = which_word(pos);
        map[j] |= make_bit(i);
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_or(usize *first, usize *last, usize *other) -> void
    {
        while (first != last) {
            *first++ |= *other++;
        }
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_xor(usize *first, usize *last, usize const *other) -> void
    {
        while (first != last) {
            *first++ ^= *other++;
        }
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_neg(usize *first, usize *last) -> void
    {
        for (; first != last; ++first) {
            *first = ~*first;
        }
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_all(usize const *first, usize const *last) -> bool 
    {
        while(first != last && *first == WORD_MAX) {
            ++first;
        }

        return first == last;
    }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    static auto bit_any(usize const *first, usize const *last) -> bool 
    {
        while(first != last && *first) {
            ++first;
        }

        return first = last;
    }
} // namespace ustl

    struct BitOps
    {
        static auto find_first() USTL_NOEXCEPT -> usize;
        static auto find_last(usize const *map, usize n) USTL_NOEXCEPT -> usize;
        static auto find_next(usize const *map, usize prev, usize n) USTL_NOEXCEPT -> usize;

        static auto clr_first_set(usize *map, usize n) USTL_NOEXCEPT -> usize;
        static auto set_first_unset(usize *map, usize n) USTL_NOEXCEPT -> usize;

        static auto left_shift(usize *map, usize n, usize k) USTL_NOEXCEPT -> void;
        static auto right_shift(usize *map, usize n, usize k) USTL_NOEXCEPT -> void;
    };

    using  details::bit_test;
    using  details::bit_flip;
    using  details::bit_clr;
    using  details::bit_set;
    using  details::bit_or;
    using  details::bit_xor;
    using  details::bit_neg;
    using  details::bit_all;
    using  details::bit_any;

} // namespace ustl

#endif // USTL_UTIL_BITOPS_H
