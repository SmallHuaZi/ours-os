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
#ifndef OURS_ARCH_X86_PERCPU_HPP
#define OURS_ARCH_X86_PERCPU_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/macro_abi.hpp>
#include <ours/cpu-local.hpp>

#include <arch/x86/descriptor.hpp>
#include <ustl/bitset.hpp>
#include <ustl/mem/container_of.hpp>

namespace ours {
    struct InterruptStatck {
        CXX11_CONSTEXPR
        static auto const kIntrStackSize = KB(16);

        FORCE_INLINE
        auto top() -> void * {
            return stack;
        }

        FORCE_INLINE
        auto bottom() -> void * {
            return stack + kIntrStackSize;
        }

        u8 stack[kIntrStackSize];
    };

    enum IntrStackOrdering {
        kDoubleFaultStack,
        kNmiStack,
        kDebugBreakStack,
        kMachineCheckStack,
        kMaxNumStacks,
    };

    struct X86Tss: arch::TaskStateSegment64 {
        typedef X86Tss  Self;

        FORCE_INLINE
        auto init() -> void {
            io_port_bitmap_base = ustl::mem::offset_of(&Self::io_bitmap);
            io_bitmap[kMaxIoBitmapBytes] = 0xff;
        }

        // We hope to use two views to access the bitmap.
        u8 io_bitmap[kMaxIoBitmapBytes + 1];
    };

    struct X86PerCpu {
        typedef X86PerCpu    Self;

        X86Tss tss alignas(16);

        InterruptStatck intr_stack[kMaxNumStacks];
    };

    /// Using CpuLocal::access<X86PerCpu>() or CpuLocal::access(&g_x86_pcpu)
    /// to access the x86 per CPU area.
    NO_MANGLE {
        CPU_LOCAL 
        inline X86PerCpu g_x86_pcpu;
    }

} // namespace ours

#endif // #ifndef OURS_ARCH_X86_PERCPU_HPP