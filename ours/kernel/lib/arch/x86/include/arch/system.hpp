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

#ifndef ARCH_X86_SYSTEM_HPP
#define ARCH_X86_SYSTEM_HPP 1

#include <arch/types.hpp>
#include <arch/sysreg.hpp>

#define X86_IMPL_SYSREG(TAG, NAME)\
    template <>\
    auto SysReg<TAG>::write(usize value) -> void\
    { asm volatile("mov %0, %%" NAME : : "r"(static_cast<usize>(value))); }      \
    template <>\
    auto SysReg<TAG>::read() -> Self \
    {\
        usize value;\
        asm volatile("mov %%" NAME ", %0" : "=r"(value));\
        return static_cast<Self>(value);\
    }

namespace arch {
    struct Cr0;
    struct Cr1;
    struct Cr2;
    struct Cr3;
    struct Cr4;

    template <>
    struct SysRegTraits<Cr0>
    {
        enum RegisterBits {
            X87,
            Sse,
            Avx,
            BndReg,
            BndCsr,
            OpMask,
            ZmmHi256,
            Hi16Zmm,
            P0,
            Pkru
        };

        typedef ustl::TypeList<
            ustl::BitField<X87, 1, usize, "x87">,
            ustl::BitField<Sse, 1, usize, "sse">,
            ustl::BitField<Avx, 1, usize, "avx">,
            ustl::BitField<BndReg, 1, usize, "bndreg">,
            ustl::BitField<BndCsr, 1, usize, "bndcsr">,
            ustl::BitField<OpMask, 1, usize, "opmask">,
            ustl::BitField<ZmmHi256, 1, usize, "zmmhi256">,
            ustl::BitField<Hi16Zmm, 1, usize, "hi16zmm">,
            ustl::BitField<P0, 1, usize, "p0">,
            ustl::BitField<Pkru, 1, usize, "pkru">
        > FieldList;
    };

    struct Cr0: public SysReg<Cr0> {};
    auto f()
    {
        auto cr0 = Cr0::read();
        cr0.set<Cr0::Avx>(0);
    }
    X86_IMPL_SYSREG(Cr0, "cr0");

    template <>
    struct SysRegTraits<Cr1>
    {
        enum {
            Vme,
            Pvi,
            Tsd,
            De,
            Pse,
            Pae,
            Mce,
            Pge,
            Pce,
            OsFxsr,
            OsMmexcpt,
            Umip,
            La57,
            Vmxe,
            Smxe,
            FsGsBase,
            Pcide,
            OsxSave,
        };
        typedef ustl::TypeList<
            ustl::BitField<0, 1, usize, "vme">,
            ustl::BitField<1, 1, usize, "pvi">,
            ustl::BitField<2, 1, usize, "tsd">,
            ustl::BitField<3, 1, usize, "de">,
            ustl::BitField<4, 1, usize, "pse">,
            ustl::BitField<5, 1, usize, "pae">,
            ustl::BitField<6, 1, usize, "mce">,
            ustl::BitField<7, 1, usize, "pge">,
            ustl::BitField<8, 1, usize, "pce">,
            ustl::BitField<9, 1, usize, "osfxsr">,
            ustl::BitField<10, 1, usize, "osmmexcpt">,
            ustl::BitField<11, 1, usize, "umip">,
            ustl::BitField<12, 1, usize, "la57">,
            ustl::BitField<13, 1, usize, "vmxe">,
            ustl::BitField<14, 1, usize, "smxe">,
            ustl::BitField<16, 1, usize, "fsgsbase">,
            ustl::BitField<17, 1, usize, "pcide">,
            ustl::BitField<18, 1, usize, "osxsave">
        > FieldList;
    };

    struct Cr1: public SysReg<Cr1> {};
    X86_IMPL_SYSREG(Cr1, "cr1");

    struct Cr2: public SysReg<Cr2> {};
    X86_IMPL_SYSREG(Cr2, "cr2");

    struct Cr3: public SysReg<Cr3> {};
    X86_IMPL_SYSREG(Cr3, "cr3");

    struct Cr4: public SysReg<Cr4> {};
    X86_IMPL_SYSREG(Cr4, "cr4");

#undef X86_SYSREG

} // namespace arch::x86

#endif // #ifndef ARCH_X86_SYSTEM_HPP