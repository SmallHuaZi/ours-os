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
#ifndef ARCH_X86_INTERRUPT_HPP
#define ARCH_X86_INTERRUPT_HPP 1

#include <arch/types.hpp>
#include <arch/macro/irqframe.hpp>
#include <arch/processor/cpu-states.hpp>
#include <ustl/util/enum_bits.hpp>

#include <ours/arch/x86/descriptor.hpp>

namespace arch {
    /// Linux IRQ vector layout.
    ///
    /// Vectors   0 ...  31 : system traps and exceptions - hardcoded events
    /// Vectors  32 ... 127 : device interrupts
    /// Vector  128         : legacy int80 syscall interface
    /// Vectors 129 ... FIRST_SYSTEM_VECTOR-1 : device interrupts
    /// Vectors FIRST_SYSTEM_VECTOR ... 255   : special interrupts
    enum class IrqVec: usize {
        DivideError = 0,
        Debug = 1,
        Nmi = 2,
        Breakpoint = 3,
        Overflow = 4,
        BoundRangeExceeded = 5,
        InvalidOpcode = 6,
        DeviceNotAvailable = 7,
        DoubleFault = 8,
        CoprocessorSegmentOverrun = 9,
        InvalidTss = 10,
        SegmentNotPresent = 11,
        StackFaultException = 12,
        GeneralProtection = 13,
        PageFault = 14,
        SpuriousInterrupt = 15,
        X87FloatingPoint = 16,
        AlignmentCheck = 17,
        MachineCheck = 18,
        SimdFloatingPoint = 19,
        Virtualizatoin = 20,
        ControlProtection = 21,

        // 22..31 are reserved for hardware, but not yet specified by Intel.
        VmmComunicationException = 31,
        IretException = 31,

        // 32..255 are only generated by `INT $n` instructions or APIC programming,
        // not by the CPU directly.
        PlatformIrqMin = 32,
        PlatformIrqMax = 0xf0 - 1,

        // 
        ApicSpurious = 0xf0,
        ApicTimer,
        ApicError,
        ApicPmi,
        IpiGeneric,
        IpiResched,
        IpiInterrupt,
        IpiSuspend,
        LastUserDefined = 255,
    };

    /// The sequence of fileds are corresponding to <arch/x86/asm.hpp>
    struct IrqFrame {
        usize di;           // 
        usize si;           // 
        usize bp;           // 
        usize bx;           // 
        usize dx;           // 
        usize cx;           // 
        usize ax;           // 
        usize r8;           // 
        usize r9;           // 
        usize r10;          // 
        usize r11;          // 
        usize r12;          // 
        usize r13;          // 
        usize r14;          // 
        usize r15;          // 
                            // 
        IrqVec vector;      //
        usize error_code;   //  |
                            //  |
        usize ip;           //  |
        usize cs;           //  |
        ArchCpuState flags; //  |

        usize usp;
        usize uss;
    };
    static_assert(offsetof(IrqFrame, di) == X86_IRQFRAME_OFFSET_DI);
    static_assert(offsetof(IrqFrame, si) == X86_IRQFRAME_OFFSET_SI);
    static_assert(offsetof(IrqFrame, bp) == X86_IRQFRAME_OFFSET_BP);
    static_assert(offsetof(IrqFrame, bx) == X86_IRQFRAME_OFFSET_BX);
    static_assert(offsetof(IrqFrame, dx) == X86_IRQFRAME_OFFSET_DX);
    static_assert(offsetof(IrqFrame, cx) == X86_IRQFRAME_OFFSET_CX);
    static_assert(offsetof(IrqFrame, ax) == X86_IRQFRAME_OFFSET_AX);
    static_assert(offsetof(IrqFrame, r8) == X86_IRQFRAME_OFFSET_R8);
    static_assert(offsetof(IrqFrame, r9) == X86_IRQFRAME_OFFSET_R9);
    static_assert(offsetof(IrqFrame, r10) == X86_IRQFRAME_OFFSET_R10);
    static_assert(offsetof(IrqFrame, r11) == X86_IRQFRAME_OFFSET_R11);
    static_assert(offsetof(IrqFrame, r12) == X86_IRQFRAME_OFFSET_R12);
    static_assert(offsetof(IrqFrame, r13) == X86_IRQFRAME_OFFSET_R13);
    static_assert(offsetof(IrqFrame, r14) == X86_IRQFRAME_OFFSET_R14);
    static_assert(offsetof(IrqFrame, r15) == X86_IRQFRAME_OFFSET_R15);
    static_assert(offsetof(IrqFrame, vector) == X86_IRQFRAME_OFFSET_VECTOR);
    static_assert(offsetof(IrqFrame, error_code) == X86_IRQFRAME_OFFSET_ERROR_CODE);
    static_assert(offsetof(IrqFrame, ip) == X86_IRQFRAME_OFFSET_IP);
    static_assert(offsetof(IrqFrame, cs) == X86_IRQFRAME_OFFSET_CS);
    static_assert(offsetof(IrqFrame, flags) == X86_IRQFRAME_OFFSET_FLAGS);
    static_assert(offsetof(IrqFrame, usp) == X86_IRQFRAME_OFFSET_USP);
    static_assert(offsetof(IrqFrame, uss) == X86_IRQFRAME_OFFSET_USS);

    struct SysCallRegs {
      usize ax;
      usize bx;
      usize cx;
      usize dx;
      usize si;
      usize di;
      usize bp;
      usize r8;
      usize r9;
      usize r10;
      usize r11;
      usize r12;
      usize r13;
      usize r14;
      usize r15;
      usize ip;
      usize flags;
      // rsp is the last field so we can simply "pop %rsp" when returning from a syscall.
      usize sp;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_INTERRUPT_HPP