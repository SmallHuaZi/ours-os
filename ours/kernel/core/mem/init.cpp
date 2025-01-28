#include <ours/mem/init.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/early.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/assert.hpp>
#include <ours/early.hpp>
#include <ours/panic.hpp>
#include <ours/status.hpp>

#include <ustl/views/span.hpp>
#include <ustl/traits/is_invocable.hpp>

#include <gktl/cpu_local.hpp>

#include <algorithm>
#include <iterator>

namespace ours::mem {
    EARLY_CODE FORCE_INLINE
    static auto alloc_node(NodeId id) -> PmNode *
    {
        auto node = EarlyMem::allocate<PmNode>(alignof(PmNode));
        if (!node) {
            panic("Fail to allocate for PmNode[{}]", id.inner);
        }
        std::construct_at(node);

        return node;
    }

    EARLY_CODE
    static auto init_node(PmNode *node) -> void
    {}


    EARLY_CODE 
    static auto init_nodes(ustl::views::Span<MemRegion> &ranges) -> void
    {
        auto start = ranges.begin(), end = ranges.end();
        auto const max_id = 10;
        for (auto nid = 0, i = 0; nid < max_id; ++nid) {
            DEBUG_ASSERT(start != end, "Node {} ownes no local memory", id);

            auto node = gktl::CpuLocal::access<PmNode>(nid);
            DEBUG_ASSERT(node != nullptr, "Cpu {} don't has its PmNode", id);

            auto const n = std::count_if(start, end, [&] (MemRegion &range) { 
                if (range.nid() == nid) {
                    return true;
                }
                return false;
            });
            DEBUG_ASSERT(n != 0, "Node {} must ownes no local memory", id);

            auto subranges = ranges.subspan(i, n);
            node->init(NodeId{ nid }, subranges);
            std::advance(start, n);
            i += n;
        }
    }

    EARLY_CODE
    auto init_pmm(ustl::views::Span<ZonePriorityInfo> zpis) -> Status
    {
        // Creates all of `PmFrame`
        MemoryModel::init(zpis);

        // Each `PmNode` extracts sets of `PmFrame` dedicated to itself
        // through accessing [FrameMapVirtAddrStart, FrameMapVirtAddrEnd]
        EarlyMem::for_each_all_regions([&] (MemRegion const &region) {
            auto nid = region.nid();
            auto node = PmNode::node(nid);
            if (!node) {
                node = alloc_node(nid);
            }
            node->attach_range(region.base, region.size);
        });

        return Status::Ok;
    }

    /// Requires:
    ///     1). 
    EARLY_CODE 
    auto init_vmm() -> Status 
    {  
        // Now, here has had the ability in allocation of `PmFrame` so
        // then we begin to initialize the kernel's descriptor of virtual 
        // address space.
        VmAspace::init_kernel_aspace();
        DEBUG_ASSERT(VmAspace::kernel_aspace() != nullptr);

        return Status::Ok;
    }
}