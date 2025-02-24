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

#ifndef ARCH_SYSREG_HPP
#define ARCH_SYSREG_HPP 1

#include <arch/types.hpp>
#include <ustl/bitfields.hpp>

namespace arch {
    template <typename RegTag>
    struct SysRegTraits;

    template <typename RegTag>
    struct SysReg
        : public SysRegTraits<RegTag>,
          public ustl::MakeBitFieldsT<typename SysRegTraits<RegTag>::FieldList>
    {
        typedef RegTag Self;
        static auto read() -> Self;
        static auto write(usize value) -> void;
    };

} // namespace arch

#endif // #ifndef ARCH_SYSREG_HPP