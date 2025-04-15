#include <ours/mem/init.hpp>

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/early-mem.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/mem/vmm.hpp>

#include <ours/assert.hpp>
#include <ours/init.hpp>
#include <ours/panic.hpp>
#include <ours/status.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/bit.hpp>
#include <ustl/limits.hpp>
#include <ustl/views/span.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/algorithms/generation.hpp>

#include <logz4/log.hpp>

#include <arch/cache.hpp>

using ustl::algorithms::min;
using ustl::algorithms::max;
using ustl::algorithms::clamp;

namespace ours::mem {
    bool g_pmm_enabled = false;

    INIT_DATA
    static Pfn s_zone_lower_pfn[NR_ZONES_PER_NODE];

    INIT_DATA
    static Pfn s_zone_upper_pfn[NR_ZONES_PER_NODE];

    INIT_CODE
    static auto set_zone_pfn_range(ustl::views::Span<Pfn> max_zone_pfn) -> void {
        auto start_pfn = phys_to_pfn(EarlyMem::min_address());
	    for (auto i = 0; i < std::size(s_zone_lower_pfn); i++) {
	    	auto end_pfn = ustl::algorithms::max(max_zone_pfn[i], start_pfn);
	    	s_zone_lower_pfn[i] = start_pfn;
	    	s_zone_upper_pfn[i] = end_pfn;
	    	start_pfn = end_pfn;
	    }

	    for (auto i = 0; i < std::size(s_zone_lower_pfn); i++) {
            log::info("Zone-{}: [0x{:X}, 0x{:X})", to_string(ZoneType(i)), s_zone_lower_pfn[i], s_zone_upper_pfn[i]);
        }
    }

    FORCE_INLINE
    static auto clamp_pfn_to_zone(Pfn pfn, ZoneType ztype) -> Pfn {  
        return clamp(pfn, s_zone_lower_pfn[ztype], s_zone_upper_pfn[ztype]);
    }

    INIT_CODE
    static auto init_node_mask(NodeMask &nodemask) -> void {
        EarlyMem::IterationContext context(bootmem::RegionType::Normal, MAX_NODES);
        while (auto region = EarlyMem::iterate(context)) {
            nodemask.set(region->nid());
        }
    }

    INIT_CODE FORCE_INLINE
    static auto dump_bootmem() -> void {
        log::debug("BootMem unused regions information"); 
        EarlyMem::IterationContext context(bootmem::RegionType::Unused, MAX_NODES);
        while (auto region = EarlyMem::iterate(context)) {
            log::debug("    None-{}| [0x{:X}, 0x{:X})", region->nid(), region->base, region->end());
        }
    }

    INIT_CODE FORCE_INLINE
    static auto alloc_node(NodeId id) -> PmNode * {
        auto node = EarlyMem::allocate<PmNode>(1, kCacheAlign, id);
        if (!node) {
            panic("Fail to allocate for PmNode[{}]", id);
        }

        return node;
    }

    INIT_CODE
    static auto create_nodes(NodeMask const &nodes) -> void {
        nodes.for_each([] (NodeId nid) {
            auto node = alloc_node(nid);
            std::construct_at(node, nid);
        });
    }

    INIT_CODE 
    static auto init_node(NodeId nid) -> usize {
        auto node = PmNode::node(nid);
        auto const [node_start_pfn, node_end_pfn] = EarlyMem::get_pfn_range_node(nid);
        node->init(node_start_pfn, node_end_pfn);

        auto zone_queues = node->zone_queues();

        auto nr_zones = 0;
        for (auto i = 0; i < NR_ZONES_PER_NODE; ++i) {
            auto const ztype = ZoneType(i);
            auto zone_start_pfn = clamp_pfn_to_zone(node_start_pfn, ztype);
            auto zone_end_pfn = clamp_pfn_to_zone(node_end_pfn, ztype);

            if (zone_start_pfn < node_start_pfn || zone_end_pfn > node_end_pfn || zone_end_pfn <= zone_start_pfn) {
                continue;
            }

            auto zone = EarlyMem::allocate<PmZone>(1, nid);
            if (!zone) {
                panic("No enough memory to place node[{}].zone[{}]", nid, to_string(ztype));
            }
            ustl::mem::construct_at(zone);

            nr_zones += 1;
            auto nr_presents = EarlyMem::count_present_frames(zone_start_pfn, zone_end_pfn);
            zone->init(nid, ztype, zone_start_pfn, zone_end_pfn, nr_presents);
            zone_queues->insert_local_zone(zone, ztype);
            set_node_state(nid, NodeStates::Type(i), true);
        }

        return nr_zones;
    }

    INIT_CODE
    static auto init_nodes(NodeMask const &nodes) -> void {
        nodes.for_each([] (NodeId nid) {
            auto const nr_zones = init_node(nid);
            log::trace("Node[{}] has {} zones", nid, nr_zones);
        });
    }

    INIT_CODE
    static auto connect_nodes(NodeMask nodes) -> void {
        nodes.for_each([&] (NodeId nid) {
            auto zq = PmNode::node(nid)->zone_queues();
            zq->connect(nodes);
            zq->dump_queue(ZoneQueues::SharedAffinity);
        });
    }

    INIT_CODE
    static auto setup_possible_nodes() -> void {
        NodeMask nodemask{};
        init_node_mask(nodemask);

        create_nodes(nodemask);

        init_nodes(nodemask);

        connect_nodes(nodemask);
    }

    INIT_CODE
    auto init_pmm(ustl::views::Span<Pfn> max_zone_pfn) -> void {
        set_zone_pfn_range(max_zone_pfn);

        setup_possible_nodes();

        init_memory_model(true);
        global_memory_model().dump();

        EarlyMem::do_handoff();

        // Ok, PMM is available now.
        g_pmm_enabled = true;

        global_node_states().for_each_online([] (NodeId nid) {
            PmNode::node(nid)->dump();
        });
    }

    INIT_CODE
    auto handoff_early_pmm(phys::MemoryHandoff &mh) -> Status {
        EarlyMem::s_bootmem = &mh.bootmem;
        mem::g_kernel_phys_base = mh.kernel_load_addr;

        mem::g_max_phys_addr = EarlyMem::s_bootmem->max_address;
        mem::g_min_phys_addr = EarlyMem::s_bootmem->min_address;

        // Verifies the signature of bootmem
        return Status::Ok; 
    }

} // namespace ours::mem