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
#ifndef ARCH_MACRO_SYSTEM_HPP
#define ARCH_MACRO_SYSTEM_HPP 1

// CR0
#define X86_CR0_PE 0x00000001  // Protected mode enable
#define X86_CR0_MP 0x00000002  // Monitor coprocessor
#define X86_CR0_EM 0x00000004  // Emulation
#define X86_CR0_TS 0x00000008  // Task switched
#define X86_CR0_ET 0x00000010  // Extension type
#define X86_CR0_NE 0x00000020  // Enable x87 exception
#define X86_CR0_WP 0x00010000  // Supervisor write protect
#define X86_CR0_NW 0x20000000  // Not write-through
#define X86_CR0_CD 0x40000000  // Cache disable
#define X86_CR0_PG 0x80000000  // Enable paging

// CR4
#define X86_CR4_PAE 0x00000020         // PAE paging
#define X86_CR4_PGE 0x00000080         // Page global enable
#define X86_CR4_OSFXSR 0x00000200      // Os supports fxsave
#define X86_CR4_OSXMMEXPT 0x00000400   // Os supports xmm exception
#define X86_CR4_UMIP 0x00000800        // User-mode instruction prevention
#define X86_CR4_VMXE 0x00002000        // Enable vmx
#define X86_CR4_FSGSBASE 0x00010000    // Enable {rd,wr}{fs,gs}base
#define X86_CR4_PCIDE 0x00020000       // Process-context ID enable 
#define X86_CR4_OSXSAVE 0x00040000     // Os supports xsave
#define X86_CR4_SMEP 0x00100000        // SMEP protection enabling
#define X86_CR4_SMAP 0x00200000        // SMAP protection enabling
#define X86_CR4_PKE 0x00400000         // Enable protection keys
#define X86_CR4_PSE 0xffffffef         // Disabling PSE bit in the CR4

// EFLAGS/RFLAGS
#define X86_FLAGS_CF (1 << 0)
#define X86_FLAGS_PF (1 << 2)
#define X86_FLAGS_AF (1 << 4)
#define X86_FLAGS_ZF (1 << 6)
#define X86_FLAGS_SF (1 << 7)
#define X86_FLAGS_TF (1 << 8)
#define X86_FLAGS_IF (1 << 9)
#define X86_FLAGS_DF (1 << 10)
#define X86_FLAGS_OF (1 << 11)
#define X86_FLAGS_STATUS_MASK (0xfff)
#define X86_FLAGS_IOPL_MASK (3 << 12)
#define X86_FLAGS_IOPL_SHIFT (12)
#define X86_FLAGS_NT (1 << 14)
#define X86_FLAGS_RF (1 << 16)
#define X86_FLAGS_VM (1 << 17)
#define X86_FLAGS_AC (1 << 18)
#define X86_FLAGS_VIF (1 << 19)
#define X86_FLAGS_VIP (1 << 20)
#define X86_FLAGS_ID (1 << 21)
#define X86_FLAGS_RESERVED_ONES 0x2
#define X86_FLAGS_RESERVED 0xffc0802a

#define X86_FLAGS_USER  \
  (X86_FLAGS_CF | X86_FLAGS_PF | X86_FLAGS_AF | X86_FLAGS_ZF | X86_FLAGS_SF | \
   X86_FLAGS_TF | X86_FLAGS_DF | X86_FLAGS_OF | X86_FLAGS_NT | X86_FLAGS_AC | \
   X86_FLAGS_ID)

#endif // #ifndef ARCH_MACRO_SYSTEM_HPP