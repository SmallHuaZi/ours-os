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
#ifndef OURS_PHYS_INIT_FS_HPP
#define OURS_PHYS_INIT_FS_HPP 1

#include <ours/types.hpp>
#include <ustl/views/span.hpp>

namespace ours::phys {
    // FIXME(SmallHuaZi) Temporary type alias for maintaining the consistency of interface.
    // It should be removed after providing the implementation of `InitFs`
    using InitFs = ustl::views::Span<u8>;
} // namespace ours::phys

#endif // #ifndef OURS_PHYS_INIT_FS_HPP 