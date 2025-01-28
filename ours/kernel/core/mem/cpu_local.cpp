#include <ours/mem/pm_node.hpp>
#include <ours/cpu_local.hpp>

#include <ustl/lazy/lazy_init.hpp>

namespace ours {
    using mem::NodeId;
    using mem::PmNode;

    template <>
    auto gktl::CpuLocal::access<PmNode>(CpuId cpuid) -> PmNode *
    {
        CPU_LOCAL
        static NodeId LOCAL_NODE_ID;

        NodeId *nid = Self::access(&LOCAL_NODE_ID, cpuid);
        return PmNode::node(*nid);
    }

    static NodeId CPU_TO_NODE[MAX_CPU_NUM];

    EARLY_CODE
    auto bind_node(CpuId cpuid, NodeId nid) -> void
    {
        CPU_TO_NODE[cpuid] = nid;
    }

} // namespace ours