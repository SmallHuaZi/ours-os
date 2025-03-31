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

    template <typename Derived>
    struct SysReg
        : SysRegTraits<Derived>,
          ustl::BitFields<ustl::bitfields::StorageUnit<usize>, typename SysRegTraits<Derived>::FieldList>
    {
        typedef Derived Self;
        typedef ustl::BitFields<ustl::bitfields::StorageUnit<usize>, typename SysRegTraits<Derived>::FieldList>  Base;
        using Base::Base;

        USTL_CONSTEXPR
        static auto read() -> Self;

        USTL_CONSTEXPR
        static auto write(Derived) -> void;

        FORCE_INLINE USTL_CONSTEXPR
        static auto from_value(usize val) -> Self {
            return {val};
        }

        FORCE_INLINE USTL_CONSTEXPR
        auto write() const -> void {
            return Self::write(*static_cast<Derived const *>(this));
        }
    };

} // namespace arch

#endif // #ifndef ARCH_SYSREG_HPP