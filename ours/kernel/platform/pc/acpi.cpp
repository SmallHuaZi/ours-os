#include <ours/platform/init.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/assert.hpp>
#include <ours/start.hpp>

#include <acpi/parser.hpp>
#include <ustl/lazy_init.hpp>

namespace ours {
    FORCE_USED
    acpi::IAcpiParser *g_acpi_parser = 0;

    FORCE_USED
    static ustl::LazyInit<acpi::AcpiParser> s_acpi_parser;

    struct PhysToVirt: public acpi::IPhysToVirt {
        virtual ~PhysToVirt() = default;

        auto phys_to_virt(PhysAddr addr, usize size) -> ktl::Result<ai_virt void const *> override {
            if (auto ptr = mem::PhysMap::phys_to_virt<void const>(addr)) {
                // TODO(SmallHuaZi) Fix the explicitly given type.
                return ustl::ok(ptr);
            }
            return ustl::err(Status::InternalError);
        }
    };
    static PhysToVirt s_p2v;

    FORCE_USED
    static auto platform_init_acpi(PhysAddr rsdp) -> void {
        DEBUG_ASSERT(!g_acpi_parser, "Duplicated calls");
        if (auto acpi_parser = acpi::AcpiParser::from_rsdp(&s_p2v, rsdp)) {
            s_acpi_parser.init(ustl::move(acpi_parser.unwrap()));
            g_acpi_parser = s_acpi_parser.data();
        }
    }

    FORCE_USED
    static auto platform_init_acpi() -> void {
        platform_init_acpi(global_handoff().acpi_rsdp);
    }
    GKTL_INIT_HOOK(AcpiInit, platform_init_acpi, PlatformInitLevel::StartUp - 1);

} // namespace ours