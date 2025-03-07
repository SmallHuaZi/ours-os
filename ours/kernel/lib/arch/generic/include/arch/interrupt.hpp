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

#ifndef ARCH_INTERRUPT_HPP
#define ARCH_INTERRUPT_HPP 1

namespace arch {
    enum class IntrTriggerMode {
        Edge = 0,
        Level = 1,
    };

    enum class IntrPolarity {
        High = 0,
        Low = 1,
    };
} // namespace arch

#endif // #ifndef ARCH_INTERRUPT_HPP