#include <ours/cpu-local.hpp>
#include <ours/irq/irq_dispatcher.hpp>

namespace ours::irq {
    CPU_LOCAL
    static IrqDispatcher IRQ_DISPATCHER;

} // namespace ours::irq