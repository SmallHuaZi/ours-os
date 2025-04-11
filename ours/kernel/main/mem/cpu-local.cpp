#include <ours/cpu-local.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_node.hpp>

namespace ours {
    using mem::NodeId;
    using mem::PmNode;

    CPU_LOCAL
    static NodeId s_nid;

    template <>
    auto CpuLocal::access<PmNode>() -> PmNode * {
        NodeId const nid = Self::read(s_nid);
        DEBUG_ASSERT(nid < MAX_NODES, "");
        return PmNode::node(nid);
    }

    template <>
    auto CpuLocal::access<PmNode>(CpuNum CpuNum) -> PmNode * {
        NodeId *nid = Self::access(&s_nid, CpuNum);
        DEBUG_ASSERT(*nid < MAX_NODES, "");
        return PmNode::node(*nid);
    }

    static NodeId s_cpu_to_node[MAX_CPU_NUM];

    INIT_CODE
    auto bind_node(CpuNum CpuNum, NodeId nid) -> void {
        s_cpu_to_node[CpuNum] = nid;
        NodeId volatile *pnid= CpuLocal::access(&s_nid, CpuNum);
        DEBUG_ASSERT(pnid, "");
        *pnid = nid;
    }

} // namespace ours
