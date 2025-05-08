#include <ours/irq/mod.hpp>
#include <ours/irq/irq_observer.hpp>
#include <ours/irq/irq_dispatcher.hpp>
#include <ours/cpu-local.hpp>

#include <ktl/new.hpp>

namespace ours::irq {
    FORCE_INLINE
    static auto get_irq_dispatcher() -> IrqDispatcher * {
        static IrqDispatcher g_irq_dispatcher;
        return &g_irq_dispatcher;
    }

    auto init_early(usize num_irqs) -> void {
        auto status = get_irq_dispatcher()->init(num_irqs);
        ASSERT(Status::Ok == status);
    }

    auto request_irq(VIrqNum virqnum, IrqHandler handler, IrqFlags flags, char const *name) -> Status {
        ASSERT(virqnum != kInvalidVIrqNum);
        return get_irq_dispatcher()->request_irq(virqnum, handler, flags, name);
    }

    auto release_irq(VIrqNum virqnum) -> Status {
        ASSERT(virqnum != kInvalidVIrqNum);
        return get_irq_dispatcher()->release_irq(virqnum);
    }

    auto handle_irq_generic(VIrqNum virqnum) -> IrqReturn {
        ASSERT(virqnum != kInvalidVIrqNum);
        return get_irq_dispatcher()->handle_irq(virqnum);
    }

} // namespace ours::irq