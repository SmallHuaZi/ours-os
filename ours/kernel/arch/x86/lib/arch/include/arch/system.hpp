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

#define X86_IMPL_SYSREG(DERIVED, NAME)\
    template <>\
    USTL_FORCEINLINE USTL_CONSTEXPR \
    auto SysReg<DERIVED>::write(DERIVED value) -> void { \
        static_assert(sizeof(DERIVED) <= sizeof(usize), \
                      "A system register is impossibly greater than the size a platform supports");\
        asm volatile("mov %0, %%" NAME : : "r"(value)); \
    } \
    template <>\
    USTL_FORCEINLINE USTL_CONSTEXPR \
    auto SysReg<DERIVED>::read() -> Self { \
        usize value;\
        asm volatile("mov %%" NAME ", %0" : "=r"(value));\
        return {{value}};\
    }

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
    using ustl::bitfields::SkipBit;

    struct Cr0;
    struct Cr2;
    struct Cr3;
    struct Cr4;

    template <>
    struct SysRegTraits<Cr0>
    {
        typedef usize   ValueType;
        enum RegisterBits {
            Pe,
            Mp,
            Em,
            Ts,
            Et,
            Ne,
            Wp,
            Am,
            Nw,
            Cd, 
            Pg,
        };

        typedef TypeList<
            Field<Id<Pe>, Name<"x87">>,
            Field<Id<Mp>, Name<"Sse">>,
            Field<Id<Em>, Name<"Avx">>,
            Field<Id<Ts>, Name<"BndReg">>,
            Field<Id<Et>, Name<"BndCsr">>,
            Field<Id<Ne>, Name<"OpMask">>,
            Field<Id<Wp>, Name<"ZmmHi256">, StartBit<16>>,
            Field<Id<Am>, Name<"Hi16Zmm">, StartBit<18>>,
            Field<Id<Nw>, Name<"P0">, StartBit<29>>,
            Field<Id<Cd>, Name<"Pkru">>,
            Field<Id<Pg>, Name<"Pkru">>
        > FieldList;
    };

    struct Cr0: public SysReg<Cr0> {};
    X86_IMPL_SYSREG(Cr0, "cr0");

    template <>
    struct SysRegTraits<Cr2> {
        typedef usize   ValueType;
        enum RegisterBits { Address };
        typedef TypeList<Field<Id<Address>, Bits<ustl::NumericLimits<VirtAddr>::DIGITS>>>
            FieldList;
    };
    struct Cr2: public SysReg<Cr2> {};
    X86_IMPL_SYSREG(Cr2, "cr2");

    template <>
    struct SysRegTraits<Cr3> {
        typedef usize   ValueType;
        enum RegisterBits { PageTableAddress, };
        typedef TypeList<Field<Id<PageTableAddress>, Bits<ustl::NumericLimits<PhysAddr>::DIGITS>>>
            FieldList;
    };
    struct Cr3: public SysReg<Cr3> { typedef SysReg<Cr3> Base; using Base::Base; };
    X86_IMPL_SYSREG(Cr3, "cr3");

    template <>
    struct SysRegTraits<Cr4>
    {
        typedef usize   ValueType;
        enum RegisterBits {
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
            Bit15Reserved,
            FsGsBase,
            Pcide,
            OsxSave,
            Bit19Reserved,
            Smep,
            Smap,
            Pke,
            Cet,
            Pks,
        };
        typedef ustl::TypeList<
            Field<Id<Vme>, Name<"vme">>,
            Field<Id<Pvi>, Name<"pvi">>,
            Field<Id<Tsd>, Name<"tsd">>,
            Field<Id<De>, Name<"de">>,
            Field<Id<Pse>, Name<"pse">>,
            Field<Id<Pae>, Name<"pae">>,
            Field<Id<Mce>, Name<"mce">>,
            Field<Id<Pge>, Name<"pge">>,
            Field<Id<Pce>, Name<"pce">>,
            Field<Id<OsFxsr>, Name<"osfxsr">>,
            Field<Id<OsMmexcpt>, Name<"osmmexcpt">>,
            Field<Id<Umip>, Name<"umip">>,
            Field<Id<La57>, Name<"la57">>,
            Field<Id<Vmxe>, Name<"vmxe">>,
            Field<Id<Smxe>, Name<"smxe">>,
            SkipBit<1>, // This bit is reserved.
            Field<Id<FsGsBase>, Name<"FsGsBase">>,
            Field<Id<Pcide>, Name<"Pcide">>,
            Field<Id<OsxSave>, Name<"osxsave">>,
            SkipBit<1>, // This bit is reserved.
            Field<Id<Smep>, Name<"smep">>,
            Field<Id<Smap>, Name<"smap">>,
            Field<Id<Pke>, Name<"pke">>,
            Field<Id<Cet>, Name<"cet">>,
            Field<Id<Pks>, Name<"pks">>
        > FieldList;
    };
    struct Cr4: public SysReg<Cr4> {};
    X86_IMPL_SYSREG(Cr4, "cr4");

    // template <>
    // struct SysRegTraits<Xcr0>
    // {
    //     typedef usize   ValueType;
    //     enum RegisterBits {
    //         X87,
    //         Sse,
    //         Avx,
    //         BndReg,
    //         BndCsr,
    //         OpMask,
    //         ZmmHi256,
    //         Hi16Zmm,
    //         P0,
    //         Pkru
    //     };

    //     typedef TypeList<
    //         Field<Id<X87>, Name<"x87">>,
    //         Field<Id<Sse>, Name<"Sse">>,
    //         Field<Id<Avx>, Name<"Avx">>,
    //         Field<Id<BndReg>, Name<"BndReg">>,
    //         Field<Id<BndCsr>, Name<"BndCsr">>,
    //         Field<Id<OpMask>, Name<"OpMask">>,
    //         Field<Id<ZmmHi256>, Name<"ZmmHi256">>,
    //         Field<Id<Hi16Zmm>, Name<"Hi16Zmm">>,
    //         Field<Id<P0>, Name<"P0">>,
    //         Field<Id<Pkru>, Name<"Pkru">>
    //     > FieldList;
    // };

    

} // namespace arch

#undef X86_SYSREG
#endif // #ifndef ARCH_X86_SYSTEM_HPP