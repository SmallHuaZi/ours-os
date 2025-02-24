#include <ours/mem/init.hpp>

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/early_mem.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/assert.hpp>
#include <ours/init.hpp>
#include <ours/panic.hpp>
#include <ours/status.hpp>
#include <ours/cpu_local.hpp>

#include <ustl/limits.hpp>
#include <ustl/views/span.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/algorithms/minmax.hpp>

#include <logz4/log.hpp>

#include <arch/cache.hpp>

using ustl::algorithms::min;
using ustl::algorithms::max;
using ustl::algorithms::clamp;

namespace ours::mem {
    INIT_DATA
    static Pfn ZONE_LOWEST_PFN[MAX_ZONES];

    INIT_DATA
    static Pfn ZONE_HIGHEST_PFN[MAX_ZONES];

    INIT_CODE
    static auto set_zone_pfn_range(ustl::views::Span<Pfn> max_zone_pfn) -> void
    {
        auto start_pfn = phys_to_pfn(EarlyMem::start_address());
	    for (auto i = 0; i < MAX_ZONES; i++) {
	    	auto end_pfn = ustl::algorithms::max(max_zone_pfn[i], start_pfn);
	    	ZONE_LOWEST_PFN[i] = start_pfn;
	    	ZONE_HIGHEST_PFN[i] = end_pfn;
	    	start_pfn = end_pfn;
	    }
    }

    FORCE_INLINE
    static auto clamp_pfn_to_zone(Pfn pfn, ZoneType ztype) -> Pfn
    {  return clamp(pfn, ZONE_LOWEST_PFN[ztype], ZONE_HIGHEST_PFN[ztype]);  }

    INIT_CODE
    static auto init_node_mask(NodeMask &nodemask) -> void
    {
        bootmem::IterationContext context(MAX_NODES, bootmem::RegionType::AllType);
        while (auto region = EarlyMem::iterate(context)) {
            nodemask.set(region->nid());
        }
    }

    INIT_CODE FORCE_INLINE
    static auto alloc_node(NodeId id) -> PmNode *
    {
        auto node = EarlyMem::allocate<PmNode>(1, arch::CACHE_SIZE, id);
        if (!node) {
            panic("Fail to allocate for PmNode[{}]", id);
        }

        return node;
    }

    INIT_CODE
    static auto create_nodes(NodeMask const &nodes) -> void
    {
        nodes.for_each([] (NodeId nid) {
            auto node = alloc_node(nid);
            std::construct_at(node, nid);
        });
    }

    INIT_CODE 
    static auto init_node(NodeId nid) -> usize
    {
        auto node = PmNode::node(nid);
        auto const [node_start_pfn, node_end_pfn] = EarlyMem::get_node_pfn_range(nid);
        node->init(node_start_pfn, node_end_pfn);

        auto zone_queues = node->zone_queues();

        auto nr_zones = 0;
        for (auto i = 0; i < NR_ZONES_PER_NODE; ++i) {
            auto const ztype = ZoneType(i);
            auto zone_start_pfn = clamp_pfn_to_zone(node_start_pfn, ztype);
            auto zone_end_pfn = clamp_pfn_to_zone(node_end_pfn, ztype);

            if (zone_start_pfn < node_start_pfn || zone_end_pfn > node_end_pfn) {
                continue;
            }

            auto zone = EarlyMem::allocate<PmZone>(1, nid);
            if (!zone) {
                panic("No enough memory to place node[{}].zone[{}]", nid, to_string(ztype));
            }

            nr_zones += 1;
            auto nr_presents = EarlyMem::count_present_frames(zone_start_pfn, zone_end_pfn);
            zone->init(nid, ztype, zone_start_pfn, zone_end_pfn, nr_presents);
            zone_queues->emplace(zone, ztype);

        }

        return nr_zones;
    }

    INIT_CODE
    static auto init_nodes(NodeMask const &nodes) -> void
    {
        nodes.for_each([] (NodeId nid) {
            auto const nr_zones = init_node(nid);
            log::trace("Node[{}] has {} zones", nid, nr_zones);
        });
    }

    INIT_CODE
    static auto connect_nodes(NodeMask nodes) -> void
    {
        nodes.for_each([&] (NodeId this_nid) {
            nodes.for_each([&] (NodeId other_nid) {
                if (this_nid == other_nid) {
                    return; 
                }

                auto this_node = PmNode::node(this_nid);
                this_node->share_with(other_nid);
            });
        });
    }

    INIT_CODE
    static auto setup_possible_nodes() -> void
    {
        NodeMask nodemask;
        init_node_mask(nodemask);

        create_nodes(nodemask);

        init_nodes(nodemask);

        connect_nodes(nodemask);
    }

    INIT_CODE
    static auto hand_off_unused_area() -> void
    {
        bootmem::IterationContext context(MAX_NODES, bootmem::RegionType::Unused);
        while (auto region = EarlyMem::iterate(context)) {

        }
    }

    INIT_CODE
    auto init_pmm(ustl::views::Span<Pfn> max_zone_pfn) -> void
    {
        MemoryModel::init();

        set_zone_pfn_range(max_zone_pfn);

        setup_possible_nodes();

        hand_off_unused_area();
    }

} // namespace ours::mem