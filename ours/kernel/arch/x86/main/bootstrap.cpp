#include <ours/arch/x86/bootstrap.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/mp.hpp>
#include <ours/platform/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/early-mem.hpp>

#include <logz4/log.hpp>

namespace ours {
    NO_MANGLE u8 const kBootstrapAreaStart[] LINK_NAME("__x86_bootstrap16_start");
    NO_MANGLE u8 const kBootstrapAreaEnd[] LINK_NAME("__x86_bootstrap16_end");
    NO_MANGLE u8 const kX86NonbootCpuEnterLongMode[] LINK_NAME("x86_nonboot_cpu_enter_long_mode");

    NO_MANGLE u8 const kDebugSymbol0[] LINK_NAME("__debug_symbol0");
    NO_MANGLE u8 const kDebugSymbol1[] LINK_NAME("__debug_symbol1");
    NO_MANGLE u8 const kDebugSymbol2[] LINK_NAME("__debug_symbol2");
    NO_MANGLE u8 const kDebugSymbol3[] LINK_NAME("__debug_symbol3");
    NO_MANGLE u8 const kDebugSymbol4[] LINK_NAME("__debug_symbol4");

    static VirtAddr s_bootstrap16_buffer_base = 0;

    INIT_CODE
    auto set_bootstrap16_buffer(VirtAddr base) -> void {
        using ustl::mem::is_aligned;
        DEBUG_ASSERT(is_aligned(s_bootstrap16_buffer_base, PAGE_SIZE));
        DEBUG_ASSERT(s_bootstrap16_buffer_base + kMinBootstrap16BufferSize <= KB(64));

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

        // At here, the trampoline code has been copied fully.

        // Let the 2nd page store data.
        auto bootstrap_data = reinterpret_cast<BootstrapData *>(bootstrap_area_start + PAGE_SIZE);
        memset(bootstrap_data, 0, sizeof(*bootstrap_data));

        bootstrap_data->lond_mode_cs = X86_GDT_KERNEL_CODE64;
        bootstrap_data->long_mode_phys_entry = s_bootstrap16_buffer_base + (kX86NonbootCpuEnterLongMode - kBootstrapAreaStart);
        bootstrap_data->long_mode_virt_entry = VirtAddr(x86_start_nonboot_cpu);

        log::trace("Bootstrap long code phys base=0x{:X}", bootstrap_data->long_mode_phys_entry);
        log::trace("Bootstrap long mode code virt base=0x{:X}", bootstrap_data->long_mode_virt_entry);

        auto gdt_base = mem::virt_to_phys(VirtAddr(x86_get_gdt()));
        if (gdt_base + kGdtSize >= GB(4)) {
            auto gdt_start = reinterpret_cast<u8 *>(ustl::mem::align_up(
                reinterpret_cast<VirtAddr>(bootstrap_data + sizeof(*bootstrap_data)), 16
            ));
            memcpy(gdt_start, x86_get_gdt(), kGdtSize);
        }
        bootstrap_data->gdtr_limit = kGdtSize - 1;
        bootstrap_data->gdtr_base = gdt_base;
        log::trace("Bootstrap GDT at 0x{:X}", gdt_base);

        {
            log::trace("Debug symbol[0] offset={:X}", kDebugSymbol0 - kBootstrapAreaStart);
            log::trace("Debug symbol[1] offset={:X}", kDebugSymbol1 - kBootstrapAreaStart);
            log::trace("Debug symbol[2] offset={:X}", kDebugSymbol2 - kBootstrapAreaStart);
            log::trace("Debug symbol[3] offset={:X}", kDebugSymbol3 - kBootstrapAreaStart);
            log::trace("Debug symbol[4] offset={:X}", kDebugSymbol4 - kBootstrapAreaStart);
        }

        auto pgd = mem::VmAspace::kernel_aspace()->arch_aspace().pgd_phys();
        if (pgd >= GB(4)) {
            // Let the 2nd page serve as temporary root page directory.
            auto temporary_pgd = bootstrap_area_start + PAGE_SIZE;
            memcpy(temporary_pgd, reinterpret_cast<u8 *>(pgd), PAGE_SIZE);
            pgd = VirtAddr(temporary_pgd);
        }
        bootstrap_data->pgd = pgd;
        *data = bootstrap_data;
        log::trace("Bootstrap PGD at 0x{:X}", pgd);

        log::trace("Bootstrap16 data at v:0x{:X}, p:0x{:X}", 
            VirtAddr(bootstrap_data), mem::PhysMap::virt_to_phys(bootstrap_data)
        );

        auto status = mem::VmAspace::kernel_aspace()->arch_aspace().map(
            s_bootstrap16_buffer_base, 
            s_bootstrap16_buffer_base, 
            kMinBootstrap16BufferSize >> PAGE_SHIFT, 
            mem::MmuFlags::PermMask | mem::MmuFlags::Discache, 
            mem::MapControl::ErrorIfExisting, 
            0
        );
        DEBUG_ASSERT(Status::Ok == status);

        status = mem::VmAspace::kernel_aspace()->arch_aspace().map(
            gdt_base,
            gdt_base,
            1, 
            mem::MmuFlags::PermMask | mem::MmuFlags::Discache, 
            mem::MapControl::ErrorIfExisting, 
            0
        );
        DEBUG_ASSERT(Status::Ok == status);

        return s_bootstrap16_buffer_base;
    }

    INIT_CODE
    static auto init_bootstrap16_buffer() -> void {
        if (s_bootstrap16_buffer_base) {
            // Avoid confliction.
            return;
        }

        auto buffer = mem::EarlyMem::allocate(kMinBootstrap16BufferSize, PAGE_SIZE, 0, KB(64), MAX_NODE);
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