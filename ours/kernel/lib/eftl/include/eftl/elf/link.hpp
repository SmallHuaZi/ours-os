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
#ifndef EFTL_ELF_LINK_HPP
#define EFTL_ELF_LINK_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/traits/decay.hpp>
#include <ustl/traits/is_invocable.hpp>

namespace eftl::elf {
    template <typename Memory, typename Relocations>
    FORCE_INLINE CXX11_CONSTEXPR
    auto relocate_relative(Memory &&memory, Relocations &relocations,
                           typename Relocations::Addr bias) -> bool {
        typedef typename Relocations::Addr      Addr;
        return relocations.visit_relative(
            [&memory, bias] (Addr addr, Addr addend = 0) -> bool {
                return memory.template write<Addr>(addr, addend + bias);
            }
        );
    }

    template <typename... Relocations>
    FORCE_INLINE CXX11_CONSTEXPR
    auto relocate(usize bias, Relocations &&...relocations) -> bool {
        static_assert(sizeof...(Relocations) > 0, "No relocations given");
        return ((relocations.visit([bias] (typename ustl::traits::DecayT<Relocations>::Addr &addr,
                                           typename ustl::traits::DecayT<Relocations>::Addr addend = 0) -> bool {
                addr += bias + addend;
                return true;
            }) || ...)
        );
    }

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_LINK_HPP