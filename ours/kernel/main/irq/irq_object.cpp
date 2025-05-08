#include <ours/irq/irq_object.hpp>

#include <ktl/new.hpp>

namespace ours::irq {
    IrqObject::IrqObject(HIrqNum hirq, VIrqNum virq, IrqFlags flags, char const *name)
        : canary_(),
          request_mutex_(),
          irqflags_(flags),
          irqdata_(hirq, virq, 0/* No chip available */),
          observers_(),
          name_(name) 
    {}

    auto IrqObject::create(HIrqNum hirq, VIrqNum virq, IrqFlags flags, char const *name) -> Self * {
        return new (mem::kGafKernel) Self(hirq, virq, flags, name);
    }

    auto IrqObject::init(IrqChip *chip) -> Status {
        irqdata_.chip = chip;
        return Status::Ok;
    }

    auto IrqObject::attach(IrqObserver &observer) -> Status {
        if (!(irqflags_ & IrqFlags::Shared) && observers_.size() > 0) {
            return Status::NoCapability;
        }

        observers_.push_back(observer);
        return Status::Ok;
    }

    auto IrqObject::detach(IrqObserver &observer) -> Status {
        observers_.erase(observers_.iterator_to(observer));
        return Status::Ok;
    }

    auto IrqObject::notify() -> void {
        for (auto &observer : observers_) {
            observer.on_event(irqdata_);
        }
    }

} // namespace ours::irq