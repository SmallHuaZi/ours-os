#include <ours/irq/init.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <arch/system.hpp>
#include <arch/interrupt.hpp>

namespace ours::irq {
    auto init_irq() -> void {
        arch::enable_interrupt();
    }

} // namespace ours::irq