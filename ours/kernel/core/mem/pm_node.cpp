#include <ours/mem/pm_node.hpp>
#include <ours/mem/pm_zone.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/early.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/panic.hpp>
#include <ours/assert.hpp>
#include <ours/cpu_local.hpp>

#include <memory>

#include <ustl/lazy/lazy_init.hpp>

namespace ours::mem {
    NodeMask    PmNode::ONLINE_NODE_MASK;
    ktl::Array<PmNode *, MAX_NR_NODES>    PmNode::ALL_NODE_LIST;
    ktl::Array<ktl::Array<usize, MAX_NR_NODES>, MAX_NR_NODES>   PmNode::NODE_DISTANCE;

    auto PmNode::init(NodeId id) -> Status
    {
        canary_.verify();
        this->allowed_gaf_ = GAF_BOOT;
        Self::ALL_NODE_LIST[id.inner] = this; 

        return Status::Ok;
    }

    auto PmNode::attach_range_unchecked(PhysAddr base, usize len, MemoryPriority priority) -> Status
    {
        if (priority == MemoryPriority::Max) {
            priority = MemoryPriority::Normal;
            for (auto const &zpi : Self::PRIORITY_PER_ZONE) {
                auto prio = zpi.get_priority_if_contains(base, len);
                if (prio != MemoryPriority::Max) {
                    priority = prio;
                }
            }
        }

        auto start_pfn = phys_to_pfn(base);
        auto end_pfn = phys_to_pfn(base + len - 1);
        auto zone = std::addressof(native_zones[usize(priority)]);
        return zone->attach_range({ start_pfn, end_pfn });

    }

    auto PmNode::alloc_frame(Gaf flags, usize order) -> ktl::Result<PmFrame *>
    {
        flags &= this->allowed_gaf_;
        return ustl::err(Status::Unimplemented);
    }

    auto PmNode::free_frame(PmFrame *frame) -> Status
    {
        return Status::Unimplemented;
    }

} // namespace ours::mem