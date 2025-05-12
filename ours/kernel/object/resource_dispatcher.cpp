#include <ours/object/resource_dispatcher.hpp>

namespace ours::object {
    ResourceDispatcher::ResourceDispatcher(RsrcKind kind, PhysAddr base, PhysAddr size, 
                                           RsrcFlags flags, Name const &name)
        : Base(), base_(base), size_(size), kind_(kind), flags_(flags), name_(name)
    {}

    auto ResourceDispatcher::create(KernelHandle<Self> *out, RsrcKind kind, PhysAddr base, 
                                    PhysAddr size, RsrcFlags flags, Name const &name) -> Status {
        if (!size) {
            return Status::InvalidArguments;
        }

        return Status::Ok;
    }
} // namespace ours::object