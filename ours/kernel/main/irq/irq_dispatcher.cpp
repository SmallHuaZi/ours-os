#include <ours/irq/irq_dispatcher.hpp>
#include <ours/mem/object-cache.hpp>

namespace ours::irq {
    auto IrqDispatcher::entrol_hwirq(HIrqNum irqnum, IrqFlags flags, IrqChip *chip, char const *name) 
        -> Status {
        // // auto object = IrqObject::create(irqnum, flags, name); // TODO: Add name
        // if (object) {
        //     return Status::Fail;
        // }

        return Status::Ok;
    }

    auto IrqDispatcher::irqnum_to_object(VIrqNum virqnum) -> IrqObject * {
        if (virqnum >= objects_.size()) {
            return nullptr;
        }

        return &objects_[virqnum];
    }

    auto IrqDispatcher::request_irq(VIrqNum irqnum, IrqHandler handler, IrqFlags flags, char const *name)
        -> Status {
        if (irqnum == kInvalidVIrqNum) {
            return Status::InvalidArguments;
        }
        if (!handler) {
            return Status::InvalidArguments;
        }

        auto object = irqnum_to_object(irqnum);
        if (!object) {
            return Status::NoResource;
        }

        

        return Status::Ok;
    }

    auto IrqDispatcher::release_irq(VIrqNum virqnum) -> Status {
        return Status::Unimplemented;
    }
} // namespace ours::irq