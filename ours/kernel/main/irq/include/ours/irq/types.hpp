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
#ifndef OURS_IRQ_TYPES_HPP
#define OURS_IRQ_TYPES_HPP 1

#include <ours/types.hpp>

namespace ours::irq {
    class IrqChip;
    class IrqObject;
    class IrqDispatcher;

    enum class IrqType {
        Edge,
        Level,
    };

    struct IrqData {
        u32 mask;
        VIrqNum virqnum;
        HIrqNum hirqnum;
        IrqChip *chip;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_TYPES_HPP