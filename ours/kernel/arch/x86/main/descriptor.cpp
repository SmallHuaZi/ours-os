#include <ours/arch/x86/descriptor.hpp>
#include <ours/cpu-local.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>
#include <arch/x86/descriptor.hpp>

namespace ours {
    struct alignas(PAGE_SIZE) Gdt {
        arch::DescPtr64 descs[X86_GDT_MAX_SELECTORS];
        arch::TaskStateSegment64 tss[MAX_CPU];

        auto load() const -> void {
            struct {
                u16 size;
                usize addr;
            } desc {
                desc.size = sizeof(*this),
                desc.addr = reinterpret_cast<usize>(this)
            };

            asm volatile("lgdt %0" :: "m"(desc) : "memory");
        }
    };

    NO_MANGLE Gdt g_gdt;
    static Gdt *s_pgdt = &g_gdt;

    auto x86_load_gdt() -> void {
        s_pgdt->load();
    }

    auto x86_setup_gdt() -> void {
        using namespace mem;

        CXX11_CONSTEXPR
        static auto kVmaFlags = VmaFlags::Read | VmaFlags::Pinned;
        ustl::Rc<VmArea> vma;
        auto result = VmAspace::kernel_aspace()->root_vma()->map_at(
            PhysMap::virt_to_phys(&g_gdt), 0, PAGE_SIZE,
            MmuFlags::Readable, VmMapOption::Commit, "k:GDT"
        );
        ASSERT(!result, "Failed to create readonly GDT");
        x86_load_gdt();
    }

    auto x86_dump_gdt() -> void {
    }

} // namespace ours