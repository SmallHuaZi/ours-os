#include <ours/platform/init.hpp>
#include <ours/mem/init.hpp>
#include <ours/arch/mem-cfg.hpp>
#include <ours/object/resource-dispatcher.hpp>

#include <ustl/array.hpp>

namespace ours {
    template <typename... LinearZoneMaxPfns>
    INIT_CODE
    static auto platform_init_pmm_with_config(ustl::TypeList<LinearZoneMaxPfns...>) -> void {
        ustl::Array<mem::Pfn, sizeof...(LinearZoneMaxPfns)>  linear_zone_pfns;
        auto handle_item = [&] <mem::ZoneType Type, usize Pfn> (mem::MaxZonePfn<Type, Pfn>) {
            linear_zone_pfns[Type] = Pfn;
        };

        (handle_item(LinearZoneMaxPfns()), ...);

        mem::init_pmm(linear_zone_pfns);
    }

    INIT_CODE
    auto platform_init_pmm() -> void {
        platform_init_pmm_with_config(ArchPmmZoneConfig());
    }

    INIT_CODE
    static auto platform_init_resource() -> void {
    }
    GKTL_INIT_HOOK(ResourceInit, platform_init_resource, gktl::InitLevel::HeapInitialized);

} // namespace ours