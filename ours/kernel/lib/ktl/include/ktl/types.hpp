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
#ifndef KTL_TYPES_HPP
#define KTL_TYPES_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>
#include <ours/mem/gaf.hpp>

namespace ktl {
    using ours::u8;
    using ours::u16;
    using ours::u32;
    using ours::u64;
    using ours::usize;

    using ours::i8;
    using ours::i16;
    using ours::i32;
    using ours::i64;
    using ours::isize;

    using ours::NodeId;
    using ours::HIrqNum;
    using ours::VIrqNum;
    using ours::Status;

    using ours::mem::Gaf;
} // namespace ktl

#endif // #ifndef KTL_TYPES_HPP