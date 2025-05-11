#include <ours/mem/vmm.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object_paged.hpp>

namespace ours::mem {
    NO_MANGLE {
        VmPage *g_zero_page;
        PhysAddr g_kernel_phys_base;
        VirtAddr g_kernel_virt_base;
    }

    auto switch_context(VmAspace *from, VmAspace *to) -> void {
        DEBUG_ASSERT(to != nullptr);
        VmAspace::switch_aspace(from, to);
    }

    auto set_active_aspace(VmAspace *aspace) -> void {
        DEBUG_ASSERT(aspace != nullptr);
        VmAspace::switch_aspace(nullptr, aspace);
    }

    auto virt_to_phys(VirtAddr virt_addr) -> PhysAddr {
        auto phys_addr = PhysMap::virt_to_phys(virt_addr);

        // TODO(SmallHuaZi): Determine if it is kernel address.

        if (!phys_addr) {
            auto status = VmAspace::kernel_aspace()->arch_aspace().query(virt_addr, &phys_addr, 0);
            if (Status::Ok != status) {
                return 0;
            }
        }

        return phys_addr;
    }

    auto phys_to_virt(PhysAddr phys_addr) -> VirtAddr {
        return PhysMap::phys_to_virt(phys_addr);
    }

} // namespace ours::mem