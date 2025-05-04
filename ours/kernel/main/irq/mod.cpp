#include <ours/irq/mod.hpp>
#include <ours/irq/irq_observer.hpp>
#include <ours/irq/irq_dispatcher.hpp>
#include <ours/cpu-local.hpp>

#include <ktl/new.hpp>

namespace ours {
    CPU_LOCAL
    static irq::IrqDispatcher g_irq_dispatcher;
    
    template <>
    FORCE_INLINE
    auto CpuLocal::access<irq::IrqDispatcher>() -> irq::IrqDispatcher * {
        return CpuLocal::access(&g_irq_dispatcher);
    }

namespace irq {
    auto request_irq(VIrqNum irqnum, IrqHandler handler, IrqFlags flags, char const *name) 
        -> Status {
        if (irqnum == kInvalidVIrqNum) {
            return Status::InvalidArguments;
        }
        return CpuLocal::access<IrqDispatcher>()->request_irq(irqnum, handler, flags, name);
    }

    auto release_irq(VIrqNum virqnum) -> Status {
        if (virqnum == kInvalidVIrqNum) {
            return Status::InvalidArguments;
        }
        return CpuLocal::access<IrqDispatcher>()->release_irq(virqnum);
    }

    auto handle_irq_generic(HIrqNum irqnum) -> Status {
        return Status::Unimplemented;
    }

} // namespace ours::irq
} // namespace ours