#include <ours/irq/mod.hpp>

namespace ours::irq {
    auto request_irq(HIrqNum irqnum, IrqHandler handler, IrqFlags flags) -> ustl::Result<VIrqNum, Status> {
        return ustl::err(Status::Unimplemented);
    }

    auto release_irq(VIrqNum virqnum) -> Status {
        return Status::Unimplemented;
    }

    auto handle_irq_generic(HIrqNum irqnum) -> Status {
        return Status::Unimplemented;
    }

} // namespace ours::irq