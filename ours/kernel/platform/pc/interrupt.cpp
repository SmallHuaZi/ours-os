#include <ours/irq/mod.hpp>
#include <arch/x86/pic.hpp>

namespace ours {
    auto platform_handle_irq(HIrqNum irqnum) -> void {
        irq::handle_irq_generic(irqnum);
    }

    static auto platform_init_apic() -> void {

    }

    static auto platform_init_irq() -> void {
        
    }

} // namespace ours
