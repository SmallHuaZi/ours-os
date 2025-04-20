#include <ours/cpu-local.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/pmm.hpp>

#include <gktl/init_hook.hpp>

namespace ours {
namespace mem {
    CPU_LOCAL
    static NodeId s_local_nid;

    CXX20_CONSTINIT 
    static ustl::Array<NodeId, MAX_CPU> s_cpu_to_node = [] () {
        decltype(s_cpu_to_node) value;
        for (auto &i: value) {
            i = MAX_NODE;
        }
        return value;
    } ();

    auto current_node() -> NodeId {
        return CpuLocal::read(s_local_nid);
    }

    auto cpu_to_node(CpuNum cpunum) -> NodeId {
        DEBUG_ASSERT(cpunum < MAX_CPU);
        return s_cpu_to_node[cpunum];
    }

    auto bind_cpu_to_node(CpuNum cpunum, NodeId nid) -> void {
        DEBUG_ASSERT(cpunum < MAX_CPU);
        DEBUG_ASSERT(nid < MAX_NODE);

        auto const onid = s_cpu_to_node[cpunum];
        if (onid != MAX_NODE) {
            if (auto node = PmNode::node(onid)) {
                node->native_cpus().set(cpunum, 0);
            }
        }

        s_cpu_to_node[cpunum] = nid;
        if (auto node = PmNode::node(nid)) {
            node->native_cpus().set(cpunum);

            // At early stage, the cpu lcoal area was created just for BP,
            // so overwritting a local nid for AP will produce a error. 
            // But CpuLocal mechanism was tied with PMM, if a instance of
            // PmNode exists in system then a local nid can be overwrited. 
            *CpuLocal::access(&s_local_nid, cpunum) = nid;
        }
    }

    INIT_CODE
    static auto register_nid_for_current_cpu() -> void {
        for_each_cpu(cpu_possible_mask(), [] (CpuNum cpu) {
            *CpuLocal::access(&s_local_nid, cpu) = cpu_to_node(cpu);
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
