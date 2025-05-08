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
#ifndef OURS_IRQ_IRQ_DOMAIN_HPP
#define OURS_IRQ_IRQ_DOMAIN_HPP 1

#include <ours/types.hpp>

namespace ours::irq {
    class IrqDomain {
        typedef IrqDomain   Self;
      public:
        static auto create_simple(VIrqNum virq_base, usize num_irqs) -> void;

        char const *name_;
        VIrqNum irq_base_;
        usize num_irqs_;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_IRQ_DOMAIN_HPP