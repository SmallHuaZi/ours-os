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
#ifndef ARCH_X86_MSR_HPP
#define ARCH_X86_MSR_HPP 1

#include <arch/types.hpp>
#include <arch/macro/msr.hpp>

namespace arch {
    // MSR identifiers.  These use the ALL_CAPS name style to be consistent with
    // the Intel manuals.  The generated header <lib/arch/x86/msr-asm.h> contains
    // macros for `MSR_<name>` so these constants can be used in assembly code.
    enum class MsrRegAddr: u32 {
        IA32ApicBase = 0x1b,
        IA32Efer = 0xc000'0080, // Extended Feature Enable Register

        IA32FsBase = 0xc000'0100,       // Current %fs.base value.
        IA32GsBase = 0xc000'0101,       // Current %gs.base value.
        IA32KernelGsBase = 0xc000'0102, // %gs.base value after `swapgs`.

        IA32SpecCtrl = 0x0000'0048, // Speculation control.
        IA32PredCmd = 0x0000'0049,  // Prediction commands.

        IA32ArchCapabilities = 0x0000'010a, // Enumeration of architectural features.

        IA32TsxCtrl = 0x0000'0122,    // TSX control.
        IA32MiscEnable = X86_MSR_IA32_MISC_ENABLE, // Miscellaneous processor features.
       
        IA32Star = X86_MSR_IA32_STAR, // 64-bit syscall entry point.
        IA32Lstar = X86_MSR_IA32_LSTAR, // 64-bit syscall entry point.
        IA32Fmask = X86_MSR_IA32_FMASK, //  

        IA32TscAux = X86_MSR_IA32_TSC_AUX, // TSC auxiliary register.

        IA32DebugCtl = 0x0000'01d9, // Debug control.

        IA32PerfCap = 0x0000'0345, // Performance monitoring features available.
        IA32TscDeadline = X86_MSR_IA32_TSC_DEADLINE, // TSC deadline 

        // Related to Last Branch Records.
        MSR_LbrSelect = 0x0000'01c8,            // Control register for the LBR feature
        MSR_LASTBRANCH_TOS = 0x0000'01c9,       // Current top of stack of LBRs.
        MSR_LASTBRANCH_0_FROM_IP = 0x0000'0680, // Source information of 0th LBR.
        MSR_LASTBRANCH_0_TO_IP = 0x0000'06c0,   // Destination information of 0th LBR.
        MSR_LBR_INFO_0 = 0x0000'0dc0,           // Additional information of 0th LBR.

        // Sparsely documented, non-architectural AMD MSRs.
        MSRC001_0015 = 0xc001'0015,       // AMD Hardware Configuration.
        MSR_VIRT_SPEC_CTRL = 0xc001'011f, // Virtualized speculation control.
        MSRC001_1020 = 0xc001'1020,       // AMD load-store configuration.
        MSRC001_1028 = 0xc001'1028,
        MSRC001_1029 = 0xc001'1029,
        MSRC001_102D = 0xc001'102d,
    };

    struct MsrIo {
        template <typename Int>
        static auto write(MsrRegAddr addr, Int value) -> void {
            // static_assert(internal::IsSupportedInt<Int>::value, "unsupported register access width");
            u64 v = reinterpret_cast<u64>(value);
            // The high-order 32 bits of each register are ignored so they need not be
            // cleared. u32 is 32 bits on x86-32 so that values will match the
            // register size, but 64 bits on x86-64 so that the compiler doesn't think
            // it needs to add an instruction to clear the high bits.
            u32 lo = static_cast<u32>(v);
            u32 hi = static_cast<u32>(v >> 32);
            asm volatile("wrmsr" : : "c"(addr), "a"(lo), "d"(hi));
        }

        template <typename Int>
        static auto read(MsrRegAddr addr) -> Int {
            // static_assert(internal::IsSupportedInt<Int>::value, "unsupported register access width");
            u32 lo, hi;
            asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(addr));
            return static_cast<Int>((static_cast<u64>(hi) << 32) | lo);
        }
    };

} // namespace arch

#endif // #ifndef ARCH_X86_MSR_HPP