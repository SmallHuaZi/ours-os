#include <ours/irq/irq_dispatcher.hpp>
#include <ours/irq/irq_object.hpp>

#include <ours/mem/object-cache.hpp>

#include <ktl/new.hpp>

namespace ours::irq {
    auto IrqDispatcher::init(usize nr_irqs) -> Status {
        auto raw_objects = new (mem::kGafKernel) IrqObject *[nr_irqs]();
        ASSERT(raw_objects);
        objects_ = ustl::views::make_span(raw_objects, nr_irqs);

        for (auto i = 0; i < nr_irqs; ++i) {
            auto irqo = IrqObject::create(i, i, IrqFlags::None, "IRQO");
            ASSERT(irqo);
            objects_[i] = irqo;
        }
        return Status::Ok;
    }

    auto IrqDispatcher::irqnum_to_object(VIrqNum virqnum) -> IrqObject * {
        if (virqnum >= objects_.size()) {
            return nullptr;
        }

        return objects_[virqnum];
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

        auto observer = new (mem::kGafKernel) IrqObserver(irqnum, flags, handler, name);
        ASSERT(observer);
        object->attach(*observer);

        return Status::Ok;
    }

    auto IrqDispatcher::release_irq(VIrqNum virqnum) -> Status {
        return Status::Unimplemented;
    }

    auto IrqDispatcher::handle_irq(VIrqNum virqnum) -> IrqReturn {
        auto object = irqnum_to_object(virqnum);
        if (!object) {
            return IrqReturn::None;
        }

        object->notify();

        return IrqReturn::Handled;
    }

} // namespace ours::irq