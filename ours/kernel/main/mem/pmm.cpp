#include <ours/mem/pm_node.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/assert.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/array.hpp>
#include <logz4/log.hpp>

namespace ours::mem {
    PhysAddr g_max_phys_addr;
    PhysAddr g_min_phys_addr;

    auto num_nodes() -> usize {
        return PmNode::num_nodes();
    }

    auto node_cpumask(NodeId nid) -> CpuMask const & {
        DEBUG_ASSERT(nid < MAX_NODE);
        return PmNode::node(nid)->native_cpus();
    }

    FORCE_INLINE
    static auto alloc_frame_internal(PmNode *prefered_node, Gaf gaf, usize order, NodeMask const &nodes) -> PmFrame * {
        DEBUG_ASSERT(prefered_node, "Given a missing node");

        auto result = prefered_node->alloc_frame(gaf, order, nodes);
        if (!result) {
            panic("Failed to allocate page with request(node: {}, order: {})", prefered_node->nid(), order);
        }

        return result.unwrap();
    }

    FORCE_INLINE
    static auto alloc_frames_internal(PmNode *prefered_node, Gaf gaf, FrameList<> *list, usize n, NodeMask const &nodes) 
        -> Status {
        DEBUG_ASSERT(prefered_node, "Given a missing node");

        auto status = prefered_node->alloc_frames(gaf, n, list, nodes);
        if (status != Status::Ok) {
            panic("Failed to allocate page with request(node: {}, order: {})", prefered_node->nid(), n);
        }

        return Status::Ok;
    }

    auto alloc_frame(Gaf gaf, usize order, NodeMask const &nodes) -> PmFrame * {
        return alloc_frame_internal(CpuLocal::access<PmNode>(), gaf, order, nodes);
    }

    auto alloc_frames(Gaf gaf, ai_out FrameList<> *list, usize n, NodeMask const &nodes) -> Status {
        return alloc_frames_internal(CpuLocal::access<PmNode>(), gaf, list, n, nodes);
    }

    auto alloc_frames(NodeId nid, Gaf gaf, ai_out FrameList<> *list, usize n, NodeMask const &nodes) -> Status {
        if (nid >= MAX_NODE) {
            return alloc_frames(gaf, list, n, nodes);
        }
        return alloc_frames_internal(PmNode::node(nid), gaf, list, n, nodes);
    }

    auto alloc_frame(NodeId nid, Gaf gaf, usize order, NodeMask const &nodes) -> PmFrame * {
        if (nid >= MAX_NODE) {
            return alloc_frame(gaf, order, nodes);
        }
        return alloc_frame_internal(PmNode::node(nid), gaf, order,  nodes);
    }

    auto free_frame(PmFrame *frame, usize order) -> void {
        frame->dump();
        PmNode::free_frame(frame, order);
    }

    auto free_frames(FrameList<> *list) -> void {
        PmNode::free_frames(list);
    }

} // namespace ours::mem