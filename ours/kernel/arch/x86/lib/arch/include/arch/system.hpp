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
    USTL_FORCEINLINE USTL_CONSTEXPR \
    auto SysReg<TAG>::write(usize value) -> void\
    { asm volatile("mov %0, %%" NAME : : "r"(static_cast<usize>(value))); }      \
    template <>\
    USTL_FORCEINLINE USTL_CONSTEXPR \
    auto SysReg<TAG>::read() -> Self \
    {\
        usize value;\
        asm volatile("mov %%" NAME ", %0" : "=r"(value));\
        return {{value}};\
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
            ustl::BitField<ustl::FieldId<X87>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<Sse>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<Avx>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<BndReg>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<BndCsr>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<OpMask>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<ZmmHi256>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<Hi16Zmm>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<P0>, ustl::FieldName<"x87">>,
            ustl::BitField<ustl::FieldId<Pkru>, ustl::FieldName<"x87">>
        > FieldList;
    };

    struct Cr0: public SysReg<Cr0> {};
    X86_IMPL_SYSREG(Cr0, "cr0");

    template <>
    struct SysRegTraits<Cr1>
    {
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
            FsGsBase,
            Pcide,
            OsxSave,
        };
        typedef ustl::TypeList<
            ustl::BitField<ustl::FieldId<Vme>, ustl::FieldName<"vme">>,
            ustl::BitField<ustl::FieldId<Pvi>, ustl::FieldName<"pvi">>,
            ustl::BitField<ustl::FieldId<Tsd>, ustl::FieldName<"tsd">>,
            ustl::BitField<ustl::FieldId<De>, ustl::FieldName<"de">>,
            ustl::BitField<ustl::FieldId<Pse>, ustl::FieldName<"pse">>,
            ustl::BitField<ustl::FieldId<Pae>, ustl::FieldName<"pae">>,
            ustl::BitField<ustl::FieldId<Mce>, ustl::FieldName<"mce">>,
            ustl::BitField<ustl::FieldId<Pge>, ustl::FieldName<"pge">>,
            ustl::BitField<ustl::FieldId<Pce>, ustl::FieldName<"pce">>,
            ustl::BitField<ustl::FieldId<OsFxsr>, ustl::FieldName<"osfxsr">>,
            ustl::BitField<ustl::FieldId<OsMmexcpt>, ustl::FieldName<"osmmexcpt">>,
            ustl::BitField<ustl::FieldId<Umip>, ustl::FieldName<"umip">>,
            ustl::BitField<ustl::FieldId<La57>, ustl::FieldName<"la57">>,
            ustl::BitField<ustl::FieldId<Vmxe>, ustl::FieldName<"vmxe">>,
            ustl::BitField<ustl::FieldId<Smxe>, ustl::FieldName<"smxe">>,
            ustl::BitField<ustl::FieldId<FsGsBase>, ustl::FieldName<"FsGsBase">>,
            ustl::BitField<ustl::FieldId<Pcide>, ustl::FieldName<"Pcide">>,
            ustl::BitField<ustl::FieldId<OsxSave>, ustl::FieldName<"osxsave">>
        > FieldList;
    };
    struct Cr1: public SysReg<Cr1> {};
    X86_IMPL_SYSREG(Cr1, "cr1");

    template <>
    struct SysRegTraits<Cr2>
    {  
        enum { Address };
        typedef ustl::TypeList<
            // TODO(SmallHuaZi) Modifies the total bits to 
            ustl::BitField<
                ustl::FieldId<Address>, 
                ustl::FieldBits<ustl::NumericLimits<VirtAddr>::DIGITS - 1>
            >
        > FieldList;
    };
    struct Cr2: public SysReg<Cr2> {};
    X86_IMPL_SYSREG(Cr2, "cr2");

    template <>
    struct SysRegTraits<Cr3>
    {  
        enum RegisterBits {
            PageTableAddress,
        };
        typedef ustl::TypeList<
            ustl::BitField<ustl::FieldId<PageTableAddress>, ustl::FieldBits<63>>
        > FieldList;  
    };
    struct Cr3: public SysReg<Cr3> {};
    X86_IMPL_SYSREG(Cr3, "cr3");

    template <>
    struct SysRegTraits<Cr4>
    {  typedef ustl::TypeList<>    FieldList;  };
    struct Cr4: public SysReg<Cr4> {};
    X86_IMPL_SYSREG(Cr4, "cr4");

#undef X86_SYSREG

} // namespace arch::x86

#endif // #ifndef ARCH_X86_SYSTEM_HPP