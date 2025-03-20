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
#ifndef OURS_TYPES_HPP
#define OURS_TYPES_HPP 1

#include <ustl/config/types.hpp>

namespace ours {
    typedef ::ustl::i8        i8;
    typedef ::ustl::i16       i16;
    typedef ::ustl::i32       i32;
    typedef ::ustl::i64       i64;
    typedef ::ustl::usize     usize;

    typedef ::ustl::u8        u8;
    typedef ::ustl::u16       u16;
    typedef ::ustl::u32       u32;
    typedef ::ustl::u64       u64;
    typedef ::ustl::isize     isize;

#ifdef OURS_CONFIG_TARGET_64BIT
    typedef ::ustl::u64     VirtAddr;
    typedef ::ustl::u64     PhysAddr;
#else
    typedef ::ustl::u32     VirtAddr;
    typedef ::ustl::u32     PhysAddr;
#endif

    typedef u32   CpuNum;
    typedef u32   HIrqNum;
    typedef u32   VIrqNum;

    /// For type check.
    struct Signals  { u32 _0; };
    struct KoId     { u32 _0; };

} // namespace ours

    // Th following is for conveniency of development now.
    typedef ::ustl::i8        i8;
    typedef ::ustl::i16       i16;
    typedef ::ustl::i32       i32;
    typedef ::ustl::i64       i64;
    typedef ::ustl::usize     usize;

    typedef ::ustl::u8        u8;
    typedef ::ustl::u16       u16;
    typedef ::ustl::u32       u32;
    typedef ::ustl::u64       u64;
    typedef ::ustl::isize     isize;

#endif // #ifndef OURS_TYPES_HPP