#include <ours/platform/init.hpp>
#include <ours/mem/init.hpp>
#include <ours/arch/mem-cfg.hpp>
#include <ours/object/resource-dispatcher.hpp>
#include <ours/mem/pmm.hpp>

#include <ustl/array.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace ours {
    INIT_CODE
    auto platform_init_pmm() -> void {
        using mem::ZoneType;
        using ustl::algorithms::min;

        mem::Pfn max_zone_pfn[NR_ZONES_PER_NODE];
        max_zone_pfn[ZoneType::Dma] = min<usize>(ARCH_MAX_DMA_PFN, mem::max_pfn());
        max_zone_pfn[ZoneType::Dma32] = min<usize>(ARCH_MAX_DMA32_PFN, mem::max_pfn());
        max_zone_pfn[ZoneType::Normal] = mem::max_pfn();

        mem::init_pmm(max_zone_pfn);
    }

    INIT_CODE
    static auto platform_init_resource() -> void {
    }
    GKTL_INIT_HOOK(ResourceInit, platform_init_resource, gktl::InitLevel::HeapInitialized);

} // namespace ours