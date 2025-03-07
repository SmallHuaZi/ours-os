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
#define OURS_MPL_CONTACT(X, Y) __OURS_MPL_CONTACT(X, Y)

#ifdef __ASSEMBLY__
#   define ULL(x)
#else
#   define ULL(x) x##ULL
#endif

/// Units
#define KB(x) ((x) * ULL(1024))
#define MB(x) KB(x * ULL(1024))
#define GB(x) MB(x * ULL(1024))
#define TB(x) GB(x * ULL(1024))
#define PB(x) TB(x * ULL(1024))

#define BIT(POS)    (ULL(1) << (POS))

#define BIT_WIDTH(N)

/// [START_BIT, END_BIT)
#define BIT_RANGE(START_BIT, END_BIT)   \
    (((ULL(1) << ((END_BIT) - (START_BIT) + ULL(1))) - ULL(1)) << (START_BIT))

#endif // #ifndef OURS_MACRO_ABI_HPP