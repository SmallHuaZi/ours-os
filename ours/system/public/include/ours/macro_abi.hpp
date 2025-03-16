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

#ifndef OURS_MACRO_ABI_HPP
#define OURS_MACRO_ABI_HPP 1

#define __OURS_MPL_CONTACT(X, Y) X ## Y
#define _OURS_MPL_CONTACT(X, Y) __OURS_MPL_CONTACT(X, Y)
#define OURS_MPL_CONTACT(X, Y)  _OURS_MPL_CONTACT(X, Y)

#ifdef __ASSEMBLY__
#   define ULL(x) x
#else
#   define ULL(x) (x##ULL)
#endif

/// Units
#define KB(x) ((x) * ULL(1024))
#define MB(x) KB(x * ULL(1024))
#define GB(x) MB(x * ULL(1024))
#define TB(x) GB(x * ULL(1024))
#define PB(x) TB(x * ULL(1024))

#define BIT(POS)    (ULL(1) << (POS))

#define BIT_RANGE(START_BIT, END_BIT)  \
    (((ULL(1) << ((END_BIT) - (START_BIT) + ULL(1))) - ULL(1)) << (START_BIT))

#define __BW_STEP(x, n) ((x) >= (ULL(1) << n) ? n + 1 : 0)
/// [START_BIT, END_BIT)
#define BIT_WIDTH(x) ( \
    __BW_STEP(x, 63) ?: __BW_STEP(x, 62) ?: __BW_STEP(x, 61) ?: __BW_STEP(x, 60) ?: \
    __BW_STEP(x, 59) ?: __BW_STEP(x, 58) ?: __BW_STEP(x, 57) ?: __BW_STEP(x, 56) ?: \
    __BW_STEP(x, 55) ?: __BW_STEP(x, 54) ?: __BW_STEP(x, 53) ?: __BW_STEP(x, 52) ?: \
    __BW_STEP(x, 51) ?: __BW_STEP(x, 50) ?: __BW_STEP(x, 49) ?: __BW_STEP(x, 48) ?: \
    __BW_STEP(x, 47) ?: __BW_STEP(x, 46) ?: __BW_STEP(x, 45) ?: __BW_STEP(x, 44) ?: \
    __BW_STEP(x, 43) ?: __BW_STEP(x, 42) ?: __BW_STEP(x, 41) ?: __BW_STEP(x, 40) ?: \
    __BW_STEP(x, 39) ?: __BW_STEP(x, 38) ?: __BW_STEP(x, 37) ?: __BW_STEP(x, 36) ?: \
    __BW_STEP(x, 35) ?: __BW_STEP(x, 34) ?: __BW_STEP(x, 33) ?: __BW_STEP(x, 32) ?: \
    __BW_STEP(x, 31) ?: __BW_STEP(x, 30) ?: __BW_STEP(x, 29) ?: __BW_STEP(x, 28) ?: \
    __BW_STEP(x, 27) ?: __BW_STEP(x, 26) ?: __BW_STEP(x, 25) ?: __BW_STEP(x, 24) ?: \
    __BW_STEP(x, 23) ?: __BW_STEP(x, 22) ?: __BW_STEP(x, 21) ?: __BW_STEP(x, 20) ?: \
    __BW_STEP(x, 19) ?: __BW_STEP(x, 18) ?: __BW_STEP(x, 17) ?: __BW_STEP(x, 16) ?: \
    __BW_STEP(x, 15) ?: __BW_STEP(x, 14) ?: __BW_STEP(x, 13) ?: __BW_STEP(x, 12) ?: \
    __BW_STEP(x, 11) ?: __BW_STEP(x, 10) ?: __BW_STEP(x, 9) ?: __BW_STEP(x, 8) ?: \
    __BW_STEP(x, 7) ?: __BW_STEP(x, 6) ?: __BW_STEP(x, 5) ?: __BW_STEP(x, 4) ?: \
    __BW_STEP(x, 3) ?: __BW_STEP(x, 2) ?: __BW_STEP(x, 1) ?: __BW_STEP(x, 0) )

#endif // #ifndef OURS_MACRO_ABI_HPP