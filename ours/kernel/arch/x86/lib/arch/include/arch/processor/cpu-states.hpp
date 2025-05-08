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
#ifndef ARCH_PROCESSOR_CPU_STATES_HPP
#define ARCH_PROCESSOR_CPU_STATES_HPP 1

#include <arch/types.hpp>
#include <arch/sysreg.hpp>
#include <ustl/bitfields.hpp>

namespace arch {
    using ustl::TypeList;
    using ustl::Field;
    using ustl::BitFields;
    using ustl::bitfields::Id;
    using ustl::bitfields::Name;
    using ustl::bitfields::Bits;
    using ustl::bitfields::Type;
    using ustl::bitfields::Enable;
    using ustl::bitfields::StartBit;

    struct ArchCpuState;

    template <>
    struct SysRegTraits<ArchCpuState> {
        enum FieldId {
            CF,
            Reserved0,
            PF,
            Reserved1,
            AF,
            Reserved2,
            ZF,
            SF,
            TF,
            IF,
            DF,
            OF, // Overflow
            IOPL, // IO pesmission level
            NT, // Nested task
            Reserved3,
            RF,
            VM,
            AC, // Alignment check
            VIF, // Virtual interrupt flags
            VIP, // Virtual interrupt process
            CpuId,  // Support cpuid.
        };

        typedef TypeList<
            Field<Id<CF>, Name<"CF">>,
            Field<Id<Reserved0>, Name<"Reserved">>,
            Field<Id<PF>, Name<"PF">>,
            Field<Id<Reserved1>, Name<"Reserved">>,
            Field<Id<AF>, Name<"AF">>,
            Field<Id<Reserved2>, Name<"Reserved">>,
            Field<Id<ZF>, Name<"ZF">>,
            Field<Id<SF>, Name<"SF">>,
            Field<Id<TF>, Name<"TF">>,
            Field<Id<IF>, Name<"IF">>,
            Field<Id<DF>, Name<"DF">>,
            Field<Id<OF>, Name<"OF">>,
            Field<Id<IOPL>, Name<"IOPL">, Bits<2>>,
            Field<Id<NT>, Name<"NT">>,
            Field<Id<Reserved3>, Name<"Reserved">>,
            Field<Id<RF>, Name<"RF">>,
            Field<Id<VM>, Name<"VM">>,
            Field<Id<AC>, Name<"AC">>,
            Field<Id<VIF>, Name<"VIF">>,
            Field<Id<VIP>, Name<"VIP">>,
            Field<Id<CpuId>, Name<"CPUID">>
        > FieldList;
    };

    struct ArchCpuState: SysReg<ArchCpuState> {};

    template <>
    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto SysReg<ArchCpuState>::write(T cpustate) -> void {
        asm ("push %0; popf" :: "r"(cpustate) : "memory");
    }

    template <>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto SysReg<ArchCpuState>::read() -> ArchCpuState {
        ArchCpuState state;
        asm ("pushf; pop %0" : "=r"(state) :: "memory");
        return state;
    }

} // namespace arch

#endif // #ifndef ARCH_PROCESSOR_CPU_STATES_HPP
