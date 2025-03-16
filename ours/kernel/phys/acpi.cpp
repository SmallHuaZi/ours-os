#include <ours/phys/acpi.hpp>

namespace ours::phys {
    struct PhysToVirtForIdentityMapping: public acpi::IPhysToVirt {
        virtual ~PhysToVirtForIdentityMapping() = default;

        auto phys_to_virt(PhysAddr addr, usize size) -> ktl::Result<ai_virt void const *> override {
            return ktl::ok(reinterpret_cast<void const *>(addr));
        }
    };

    static PhysToVirtForIdentityMapping S_P2V;

    auto make_apic_parser(PhysAddr rsdp) -> ktl::Result<acpi::AcpiParser> {
        return acpi::AcpiParser::from_rsdp(&S_P2V, rsdp);
    }

} // namespace ours::phys