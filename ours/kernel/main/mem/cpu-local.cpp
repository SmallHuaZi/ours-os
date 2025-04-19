#include <ours/cpu-local.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/pmm.hpp>

#include <gktl/init_hook.hpp>

namespace ours {
namespace mem {
    CPU_LOCAL
    static NodeId s_local_nid;

    auto current_node() -> NodeId {
        return CpuLocal::read(s_local_nid);
    }

    static auto register_nid_for_current_cpu() -> void {
        for_each_cpu(cpu_possible_mask(), [] (CpuNum cpu) {
            *CpuLocal::access(&s_local_nid, cpu) = mem::cpu_to_node(cpu);
        });
    }
    // When cpu local area has been setup, we should install local node id to enable the version of 
    // `alloc_frame` without specifing explicitly `nid`.
    GKTL_INIT_HOOK(RegisterNid, register_nid_for_current_cpu, gktl::InitLevel::CpuLocal + 1);
}
    using mem::PmNode;

    template <>
    auto CpuLocal::access<PmNode>() -> PmNode * {
        NodeId const nid = Self::read(mem::s_local_nid);
        DEBUG_ASSERT(nid < MAX_NODE, "");
        return PmNode::node(nid);
    }

} // namespace ours
