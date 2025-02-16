///
///
///
///
///
///
///
///
///
///
///
///

#include <ours/mem/init.hpp>

#include <ours/mem/pm_zone.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/early.hpp>
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
    struct PhysMemInit
    {
        static auto get() -> PhysMemInit & 
        {  return PM_INIT;  }

        auto init(bootmem::IBootMem *bootmem, ustl::views::Span<Pfn> max_zone_pfn) -> void;

        auto clamp_pfn_to_zone(Pfn pfn, ZoneType ztype) -> Pfn
        {  return clamp(pfn, zone_lowest_pfn_[ztype], zone_highest_pfn_[ztype]); }

        auto get_node_pfn_range(NodeId nid) -> gktl::Range<Pfn>
        {  return bootmem_->get_pfn_range_for_nid(nid);  }

        auto count_present_frames(Pfn start, Pfn end) -> usize
        {  return bootmem_->count_present_frames(start, end, MAX_NODES);  }

    private:
        auto set_zone_pfn_range(ustl::views::Span<Pfn> max_zone_pfn) -> void;

        Pfn zone_lowest_pfn_[MAX_ZONES];
        Pfn zone_highest_pfn_[MAX_ZONES];
        bootmem::IBootMem *bootmem_;

        INIT_DATA
        static PhysMemInit  PM_INIT;
    };

    auto PhysMemInit::init(bootmem::IBootMem *bootmem, ustl::views::Span<Pfn> max_zone_pfn) -> void
    {
        this->bootmem_ = bootmem;
        set_zone_pfn_range(max_zone_pfn);
    }

    auto PhysMemInit::set_zone_pfn_range(ustl::views::Span<Pfn> max_zone_pfn) -> void
    {
        auto start_pfn = phys_to_pfn(bootmem_->start_address());
	    for (auto i = 0; i < MAX_ZONES; i++) {
	    	auto end_pfn = ustl::algorithms::max(max_zone_pfn[i], start_pfn);
	    	zone_lowest_pfn_[i] = start_pfn;
	    	zone_highest_pfn_[i] = end_pfn;
	    	start_pfn = end_pfn;
	    }
    }

    INIT_CODE 
    static auto init_node(NodeId nid) -> usize
    {
        auto &early_mem = PhysMemInit::get();

        auto node = PmNode::node(nid);
        auto const [node_start_pfn, node_end_pfn] = early_mem.get_node_pfn_range(nid);
        node->init(node_start_pfn, node_end_pfn);

        auto zone_queues = node->zone_queues();

        auto nr_zones = 0;
        for (auto i = 0; i < NR_ZONES_PER_NODE; ++i) {
            auto const ztype = ZoneType(i);
            auto zone_start_pfn = early_mem.clamp_pfn_to_zone(node_start_pfn, ztype);
            auto zone_end_pfn = early_mem.clamp_pfn_to_zone(node_end_pfn, ztype);

            if (zone_start_pfn < node_start_pfn || zone_end_pfn > node_end_pfn) {
                continue;
            }

            auto zone = EarlyMem::allocate<PmZone>(nid, 1);
            if (!zone) {
                panic("No enough memory to place node[{}].zone[{}]", nid, to_string(ztype));
            }

            nr_zones += 1;
            auto nr_presents = early_mem.count_present_frames(zone_start_pfn, zone_end_pfn);
            zone->init(nid, ztype, zone_start_pfn, zone_end_pfn, nr_presents);
            zone_queues->emplace(zone, ztype);

        }

        return nr_zones;
    }

    INIT_CODE FORCE_INLINE
    static auto alloc_node(NodeId id) -> PmNode *
    {
        auto node = EarlyMem::allocate<PmNode>(id, 1, arch::CACHE_SIZE);
        if (!node) {
            panic("Fail to allocate for PmNode[{}]", id);
        }

        return node;
    }

    INIT_CODE
    static auto create_all_possible_nodes() -> void
    {
        EarlyMem::for_each_all_regions([] (usize, usize, NodeId nid) {
            NodeStates::set_possible(nid);
        });

        NodeStates::for_each_possible([] (NodeId nid) {
            auto node = alloc_node(nid);
            std::construct_at(node, nid);
        });
    }

    INIT_CODE
    auto init_pmm(bootmem::IBootMem *bootmem, ustl::views::Span<Pfn> max_zone_pfn) -> Status
    {
        PhysMemInit::get().init(bootmem, max_zone_pfn);

        VmAspace::init_kernel_aspace();
        auto kaspace = VmAspace::kernel_aspace();

        MemoryModel::init();

        create_all_possible_nodes();

        // Initialize all possible nodes.
        NodeStates::for_each_possible([] (NodeId nid) {
            auto const nr_zones = init_node(nid);
            log::trace("Node[{}] has {} zones", nid, nr_zones);
        });

        return Status::Ok;
    }

    /// Requires:
    ///     1). 
    INIT_CODE 
    auto init_vmm() -> Status
    {  
        // Now, here has had the ability in allocation of `PmFrame` so
        // then we begin to initialize the kernel's descriptor of virtual 
        // address space.

        return Status::Ok;
    }
}