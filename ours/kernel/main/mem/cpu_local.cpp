#include <ours/cpu_local.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_node.hpp>

namespace ours {
    using mem::NodeId;
    using mem::PmNode;

    CPU_LOCAL
    static NodeId LOCAL_NODE_ID;

    template <>
    auto CpuLocal::access<PmNode>(CpuNum CpuNum) -> PmNode *
    {
        NodeId *nid = Self::access(&LOCAL_NODE_ID, CpuNum);
        return PmNode::node(*nid);
    }

    static NodeId CPU_TO_NODE[MAX_CPU_NUM];

    INIT_CODE
    auto bind_node(CpuNum CpuNum, NodeId nid) -> void
    {
        CPU_TO_NODE[CpuNum] = nid;
        NodeId volatile *nid_ptr = CpuLocal::access(&LOCAL_NODE_ID, CpuNum);
    }

} // namespace ours
