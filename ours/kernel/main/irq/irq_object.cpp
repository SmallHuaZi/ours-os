#include <ours/irq/irq_object.hpp>
#include <ours/mem/object-cache.hpp>

namespace ours::irq {
    auto IrqObject::create(HIrqNum hirq, VIrqNum virq, IrqFlags flags, char const *name) 
        -> ustl::Rc<Self> {
        
    }

    IrqObject::IrqObject(HIrqNum irqnum, VIrqNum virq, IrqFlags flags, char const *name)
        : canary_(),
          request_mutex_(),
          data_(),
          observers_(),
          name_(name) {
    }
} // namespace ours::irq