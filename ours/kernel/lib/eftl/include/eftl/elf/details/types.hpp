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
#ifndef EFTL_ELF_DETAILS_TYPES_HPP
#define EFTL_ELF_DETAILS_TYPES_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/macro_abi.hpp>

namespace eftl::elf {
    typedef u32 Elf32Addr;
    typedef u32 Elf32Off;
    typedef u16 Elf32Half;
    typedef u32 Elf32Word;
    typedef i32 Elf32Sword;

    typedef u64 Elf64Addr;
    typedef u64 Elf64Off;
    typedef u16 Elf64Half;
    typedef u32 Elf64Word;
    typedef i32 Elf64Sword;

    typedef u64 Elf64Xword;
    typedef i64 Elf64Sxword;

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_DETAILS_TYPES_HPP