#include <ours/arch/x86/bootstrap.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/platform/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/early-mem.hpp>

namespace ours {
    NO_MANGLE u8 const kBootstrapAreaStart[] LINK_NAME("__x86_bootstrap16_start");
    NO_MANGLE u8 const kBootstrapAreaEnd[] LINK_NAME("__x86_bootstrap16_end");

    static VirtAddr s_bootstrap16_buffer_base = 0;

    INIT_CODE
    auto set_bootstrap16_buffer(VirtAddr base) -> void {
        using ustl::mem::is_aligned;
        DEBUG_ASSERT(is_aligned(s_bootstrap16_buffer_base, PAGE_SIZE));
        DEBUG_ASSERT(s_bootstrap16_buffer_base + kMinBootstrap16BufferSize <= MB(1));

        s_bootstrap16_buffer_base = base;
    }

    INIT_CODE
    auto make_bootstrap_area(BootstrapData **data) -> PhysAddr {
        DEBUG_ASSERT(data);
        DEBUG_ASSERT(s_bootstrap16_buffer_base, "There is no a given bootstrap area");

        CXX11_CONSTEXPR
        static auto const kGdtSize = X86_GDT_MAX_SELECTORS * X86_SEGMENT_SELECTOR_SIZE;

        auto const area_size = kBootstrapAreaEnd - kBootstrapAreaStart;
        DEBUG_ASSERT(area_size < PAGE_SIZE * 2);

        auto bootstrap_area_start = mem::PhysMap::phys_to_virt<u8>(s_bootstrap16_buffer_base);
        memcpy(bootstrap_area_start, kBootstrapAreaStart, area_size);

        auto bootstrap_data = reinterpret_cast<BootstrapData *>(ustl::mem::align_up(
            reinterpret_cast<VirtAddr>(bootstrap_area_start + area_size), 8
        ));
        // For constrcuting dummy relative addressing.
        bootstrap_data->bootstrap_entry = mem::PhysMap::virt_to_phys(bootstrap_area_start);

        auto gdt_base = mem::virt_to_phys(VirtAddr(x86_get_gdt()));
        if (gdt_base + kGdtSize >= GB(4)) {
            auto gdt_start = reinterpret_cast<u8 *>(ustl::mem::align_up(
                reinterpret_cast<VirtAddr>(bootstrap_data + sizeof(*bootstrap_data)), 16
            ));
            memcpy(gdt_start, x86_get_gdt(), kGdtSize);
        }
        bootstrap_data->gdtr.limit = kGdtSize;
        bootstrap_data->gdtr.base = gdt_base;

        auto pgd = mem::VmAspace::kernel_aspace()->arch_aspace().pgd_phys();
        if (pgd >= GB(4)) {
            // Let the 2nd page serve as temporary root page directory.
            auto temporary_pgd = bootstrap_area_start + PAGE_SIZE;
            memcpy(temporary_pgd, reinterpret_cast<u8 *>(pgd), PAGE_SIZE);
            pgd = VirtAddr(temporary_pgd);
        }
        bootstrap_data->pgd = pgd;

        return s_bootstrap16_buffer_base;
    }

    INIT_CODE
    static auto init_bootstrap16_buffer() -> void {
        if (s_bootstrap16_buffer_base) {
            // Avoid confliction.
            return;
        }

        auto buffer = mem::EarlyMem::allocate(kMinBootstrap16BufferSize, PAGE_SIZE, 0, MB(1), MAX_NODE);
        ASSERT(buffer, "Failed to allocate APs's bootstrap buffer");

        set_bootstrap16_buffer(mem::PhysMap::virt_to_phys(buffer));
    }
    GKTL_INIT_HOOK(InitBootstrap16Buffer, init_bootstrap16_buffer, PlatformInitLevel::PrePmm);

    INIT_CODE
    static auto reclaim_bootstrap16_buffer() -> void {
        for (auto offset = 0; offset < kMinBootstrap16BufferSize; offset += PAGE_SIZE) {
            auto frame = mem::phys_to_frame(s_bootstrap16_buffer_base + offset);
            mem::free_frame(frame, 0);
        }
    }
    GKTL_INIT_HOOK(ReclaimBootstrap16Buffer, reclaim_bootstrap16_buffer, gktl::InitLevel::Platform);

} // namespace ours