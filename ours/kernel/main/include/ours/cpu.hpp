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

#ifndef OURS_CPU_HPP
#define OURS_CPU_HPP

#if !__has_include(<ours/arch/cpu.hpp>)
#   error "The header <ours/arch/cpu.hpp> is required by the protocol [kernel.cpu]"
#endif

#include <ours/cpu_cfg.hpp>
#include <ours/arch/cpu.hpp>

namespace ours {
    struct Cpu
    {

    };

} // namespace ours

#endif // #ifndef OURS_CORE_CPU_HPP