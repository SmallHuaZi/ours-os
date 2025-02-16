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
    typedef ::ustl::isize     isize;

    typedef ::ustl::u8        u8;
    typedef ::ustl::u16       u16;
    typedef ::ustl::u32       u32;
    typedef ::ustl::u64       u64;
    typedef ::ustl::usize     usize;

    typedef usize   CpuId;
    typedef usize   VirtAddr;
    typedef usize   PhysAddr;

    /// For type check.
    struct Handle   { usize _0; };
    struct Signal   { usize _0; };
    struct KoId     { usize _0; };

} // namespace ours

#endif // #ifndef OURS_TYPES_HPP