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
#ifndef OURS_IRQ_IRQ_CHIP_HPP
#define OURS_IRQ_IRQ_CHIP_HPP 1

#include <ours/cpu_mask.hpp>
#include <ours/status.hpp>
#include <ours/irq/types.hpp>

namespace ours::irq {
    struct IrqChip {
        virtual auto startup(IrqData &data) -> void {}
        virtual auto teawdown(IrqData &data) -> void {}

        virtual auto enable(IrqData &data) -> void {}
        virtual auto disable(IrqData &data) -> void {}

        virtual auto mask(IrqData &data) -> void = 0;
        virtual auto unmask(IrqData &data) -> void = 0;
        virtual auto send_eoi(IrqData &data) -> void {}
        virtual auto send_ack(IrqData &data) -> void {}

        virtual auto set_affinity(IrqData &data, CpuMask *dest, bool force) -> Status;
        virtual auto retrigger(IrqData &data) -> int;
        virtual auto set_type(IrqData &data, unsigned int flow_type) -> int;
        virtual auto set_wake(IrqData &data, unsigned int on) -> int;

        virtual auto lock_bus(IrqData &data) -> void;
        virtual auto sync_unlock_bus(IrqData &data) -> void;

        virtual auto suspend(IrqData &data) -> void;
        virtual auto resume(IrqData &data) -> void;

        virtual auto send_ipi(IrqData &data, CpuNum cpu) -> void {}

        auto send_ipi(IrqData &data, CpuMask const &dest) -> void {
            for_each_cpu(dest, [&] (CpuNum cpunum) {
                this->send_ipi(data, cpunum);
            });
        }

        virtual auto setup_nmi(IrqData &data) -> Status {
            return Status::Unsupported;
        }
        virtual auto teardown_nmi(IrqData &data) -> void {}

      protected:
        char const *name_;
    };

} // namespace ours::irq

#endif // #ifndef OURS_IRQ_CHIP_HPP