#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/x86/percpu.hpp>

#include <ours/cpu-local.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_mapping.hpp>

#include <arch/x86/descriptor.hpp>
#include <ustl/mem/container_of.hpp>

namespace ours {
    struct alignas(PAGE_SIZE) PACKED Gdt {
        auto load() const -> void {
            arch::DescPtr desc {
                .limit = sizeof(*this) - 1,
                .base = reinterpret_cast<usize>(this)
            };

            arch::lgdt(desc);
        }

        FORCE_INLINE
        auto tss_selector(CpuNum cpunum) const -> u16 {
            return (X86_GDT_MAX_SELECTORS + cpunum) * sizeof(arch::GdtDesc);
        }

        FORCE_INLINE
        auto get_tss_desc(CpuNum cpunum) -> arch::GdtDesc & {
            return descs[X86_GDT_MAX_SELECTORS + cpunum];
        }

        ustl::Array<arch::GdtDesc, X86_GDT_MAX_SELECTORS + MAX_CPU> descs;
    };

    NO_MANGLE Gdt g_gdt;
    static Gdt *s_pgdt = &g_gdt;

    auto x86_load_gdt() -> void {
        s_pgdt->load();
    }

    auto x86_setup_gdt() -> void {
        using namespace mem;

        VirtAddr virt_addr = 0;
        ustl::Rc<VmArea> vma;
        auto result = VmAspace::kernel_aspace()->root_vma()->map_at(
            PhysMap::virt_to_phys(&g_gdt), 
            &virt_addr, // Request the virtual address
            PAGE_SIZE,
            MmuFlags::Readable, // Read only
            VmMapOption::Commit | VmMapOption::Pinned, // Disable swapping out.
            "k:GDT"
        );
        ASSERT(!result, "Failed to create readonly GDT");

        s_pgdt = reinterpret_cast<decltype(s_pgdt)>(virt_addr);
        x86_load_gdt();
    }

    static auto dump_desc(arch::GdtDesc const &desc) -> void {
        log::info("{: <10} | 0x{:0<16X} | 0x{:<16X} | {}",
            to_string(desc.type()),
            desc.base(),
            desc.limit(),
            to_string(desc.dpl())
        );
    }

    auto x86_dump_gdt() -> void {
        auto &desc = s_pgdt->descs;
        log::info("{: <10} | {: <16} | {: <16} | DPL", "Type", "Base", "Limit");
        for (auto i = 0; i < desc.size(); ++i) {
            dump_desc(desc[i]);
        }
    }

    /// Code bottom is about TSS
    auto x86_init_tss_percpu() -> void {
        auto const cpunum = arch_current_cpu();
        DEBUG_ASSERT(cpunum < MAX_CPU, "Invalid CPU number: {}", cpunum);

        auto tss = &CpuLocal::access(&g_x86_pcpu)->tss;
        tss->init();

        // Do not use s_pdgt, because it is readonly.
        g_gdt.get_tss_desc(cpunum).set_base(usize(tss))
             .set_limit(sizeof(X86Tss) - 1)
             .set_type(arch::SegType::Tss)
             .set_present(true)
             .set_dpl(arch::Dpl::Ring0);
        dump_desc(g_gdt.get_tss_desc(cpunum));
        
        arch::ltr(g_gdt.tss_selector(cpunum));
    }

} // namespace ours