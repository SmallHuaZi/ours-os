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
#ifndef ARCH_MACRO_DEBUG_HPP
#define ARCH_MACRO_DEBUG_HPP 1

#include <ours/macro_abi.hpp>

// DR6
#define X86_DR6_B0 (ULL(1) << 0)
#define X86_DR6_B1 (ULL(1) << 1)
#define X86_DR6_B2 (ULL(1) << 2)
#define X86_DR6_B3 (ULL(1) << 3)
#define X86_DR6_BD (ULL(1) << 13)
#define X86_DR6_BS (ULL(1) << 14)
#define X86_DR6_BT (ULL(1) << 15)

// NOTE: DR6 is used as a read-only status registers, and it is not writeable through userspace.
//       Any bits attempted to be written will be ignored.
#define X86_DR6_USER_MASK \
  (X86_DR6_B0 | X86_DR6_B1 | X86_DR6_B2 | X86_DR6_B3 | X86_DR6_BD | X86_DR6_BS | X86_DR6_BT)
/* Only bits in X86_DR6_USER_MASK are writeable.
 * Bits 12 and 32:63 must be written with 0, the rest as 1s */
#define X86_DR6_MASK (0xffff0ff0ul)

// DR7
#define X86_DR7_L0 (ULL(1) << 0)
#define X86_DR7_G0 (ULL(1) << 1)
#define X86_DR7_L1 (ULL(1) << 2)
#define X86_DR7_G1 (ULL(1) << 3)
#define X86_DR7_L2 (ULL(1) << 4)
#define X86_DR7_G2 (ULL(1) << 5)
#define X86_DR7_L3 (ULL(1) << 6)
#define X86_DR7_G3 (ULL(1) << 7)
#define X86_DR7_LE (ULL(1) << 8)
#define X86_DR7_GE (ULL(1) << 9)
#define X86_DR7_GD (ULL(1) << 13)
#define X86_DR7_RW0 (ULL(3) << 16)
#define X86_DR7_LEN0 (ULL(3) << 18)
#define X86_DR7_RW1 (ULL(3) << 20)
#define X86_DR7_LEN1 (ULL(3) << 22)
#define X86_DR7_RW2 (ULL(3) << 24)
#define X86_DR7_LEN2 (ULL(3) << 26)
#define X86_DR7_RW3 (ULL(3) << 28)
#define X86_DR7_LEN3 (ULL(3) << 30)

// NOTE1: Even though the GD bit is writable, we disable it for the write_state syscall because it
//        complicates a lot the reasoning about how to access the registers. This is because
//        enabling this bit would make any other access to debug registers to issue an exception.
//        New syscalls should be define to lock/unlock debug registers.
// NOTE2: LE/GE bits are normally ignored, but the manual recommends always setting it to 1 in
//        order to be backwards compatible. Hence they are not writable from userspace.
#define X86_DR7_USER_MASK                                                                     \
  (X86_DR7_L0 | X86_DR7_G0 | X86_DR7_L1 | X86_DR7_G1 | X86_DR7_L2 | X86_DR7_G2 | X86_DR7_L3 | \
   X86_DR7_G3 | X86_DR7_RW0 | X86_DR7_LEN0 | X86_DR7_RW1 | X86_DR7_LEN1 | X86_DR7_RW2 |       \
   X86_DR7_LEN2 | X86_DR7_RW3 | X86_DR7_LEN3)

#endif // #ifndef ARCH_MACRO_DEBUG_HPP