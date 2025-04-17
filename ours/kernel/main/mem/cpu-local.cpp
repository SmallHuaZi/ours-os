#include <ours/cpu-local.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/pmm.hpp>

#include <gktl/init_hook.hpp>

namespace ours {
    using mem::PmNode;

    CPU_LOCAL
    static NodeId s_local_nid;

    template <>
    auto CpuLocal::access<PmNode>() -> PmNode * {
        NodeId const nid = Self::read(s_local_nid);
        DEBUG_ASSERT(nid < MAX_NODE, "");
        return PmNode::node(nid);
    }

    static auto register_nid_for_current_cpu() -> void {
        CpuLocal::write(s_local_nid, mem::cpu_to_node(arch_current_cpu()));
    }
    // When cpu local area has been setup, we should install local node id to enable the version of 
    // `alloc_frame` without specifing explicitly `nid`.
    GKTL_INIT_HOOK(RegisterNid, register_nid_for_current_cpu, gktl::InitLevel::CpuLocal + 1);

} // namespace ours
