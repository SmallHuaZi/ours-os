#include <ours/platform/init.hpp>
#include <ours/mem/init.hpp>
#include <ours/arch/mem-cfg.hpp>
#include <ours/object/resource-dispatcher.hpp>
#include <ours/mem/pmm.hpp>

#include <ustl/array.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace ours {
    template <typename>
    struct DisabledZoneFilter;

    template <mem::ZoneType Type, usize Pfn, bool Enable>
    struct DisabledZoneFilter<mem::MaxZonePfn<Type, Pfn, Enable>> 
        : ustl::traits::BoolConstant<!Enable> 
    {};

    /// This template design ensures that changes to the number of zone types do not affect existing implementations.
    template <typename... LinearZoneMaxPfns>
    INIT_CODE
    static auto platform_init_pmm_with_config(ustl::TypeList<LinearZoneMaxPfns...>) -> void {
        static_assert(sizeof...(LinearZoneMaxPfns) <= NR_ZONES_PER_NODE, "To many zones per node");

        ustl::Array<mem::Pfn, sizeof...(LinearZoneMaxPfns)>  linear_zone_pfns;
        auto index = 0;
        auto handle_item = [&] <mem::ZoneType Type, usize Pfn, bool Enable> (mem::MaxZonePfn<Type, Pfn, Enable>) {
            static_assert(Enable, "There is a disabled zone type");
            linear_zone_pfns[index++] = ustl::algorithms::min(Pfn, mem::max_pfn());
        };
 
        (handle_item(LinearZoneMaxPfns()), ...);
 
        mem::init_pmm(linear_zone_pfns);
    }
 
    INIT_CODE
    auto platform_init_pmm() -> void {
        typedef ustl::TypeAlgos::Filter<ArchPmmZoneConfig, DisabledZoneFilter>   ZoneConfig;
        platform_init_pmm_with_config(ZoneConfig());
    }

    INIT_CODE
    static auto platform_init_resource() -> void {
    }
    GKTL_INIT_HOOK(ResourceInit, platform_init_resource, gktl::InitLevel::Heap);

} // namespace ours