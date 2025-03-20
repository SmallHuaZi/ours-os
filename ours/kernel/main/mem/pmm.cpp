#include <ours/mem/pm_node.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/assert.hpp>

#include <ours/cpu_local.hpp>

namespace ours::mem {
    auto alloc_frame(Gaf flags, usize order) -> PmFrame *
    {
        auto local_node = CpuLocal::access<PmNode>();
        DEBUG_ASSERT(local_node != nullptr);
        if (auto result = local_node->alloc_frame(flags, order)) {
            return result.unwrap();
        }

        return nullptr;
    }

    auto alloc_frame(Gaf flags, ai_out PhysAddr *phys_addr, usize order) -> PmFrame *
    {
        auto local_node = CpuLocal::access<PmNode>();
        DEBUG_ASSERT(local_node != nullptr);
        if (auto result = local_node->alloc_frame(flags, order)) {
            auto frame = result.unwrap();
            if (phys_addr) {
                *phys_addr =  MemoryModel::frame_to_phys(frame);
            }
        }

        return nullptr;
    }

    auto alloc_frame_on_node(NodeId nid, Gaf flags, usize order) -> PmFrame *
    {
        DEBUG_ASSERT(nid < MAX_NODES);
        if (auto node = PmNode::node(nid)) {
            if (auto result = node->alloc_frame(flags | Gaf::OnlyThisNode, order)) {
                return result.unwrap(); 
            }
        }

        return nullptr;
    }

    auto free_frame(PmFrame *frame, usize order) -> void
    {
        auto node = PmNode::node(0);
        DEBUG_ASSERT(node);
        return node->free_frame(frame, order);
    }

    auto alloc_frames(Gaf flags, ai_out FrameList<> *list, usize n) -> Status
    {
        auto local_node = CpuLocal::access<PmNode>();
        DEBUG_ASSERT(local_node != nullptr);
        return local_node->alloc_frames(flags, n, list);
    }

    auto free_frames(FrameList<> *list) -> void
    {
        auto local_node = CpuLocal::access<PmNode>();
        DEBUG_ASSERT(local_node != nullptr);
        return local_node->free_frames(list);
    }

} // namespace ours::mem