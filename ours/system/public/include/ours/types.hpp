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

    typedef usize     VirtAddr;
    typedef usize     PhysAddr;
    typedef usize     PgOff;

    typedef u32   CpuNum;
    typedef u32   HIrqNum;
    typedef u32   VIrqNum;
    typedef u32   NodeId;
    typedef usize Ticks;

    /// For type check.
    struct CpuSet   { u32 _0; };
    struct KoId     { u32 _0; };

    // enum class AlignVal: usize {};
    typedef usize   AlignVal;

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