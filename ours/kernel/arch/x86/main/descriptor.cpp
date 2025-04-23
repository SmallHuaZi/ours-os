#include <ours/arch/x86/descriptor.hpp>
#include <ours/cpu-local.hpp>
#include <ours/mem/vm_aspace.hpp>
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
        // reload here.
        ustl::Rc<VmArea> gdtvma;
        auto status = VmAspace::kernel_aspace()
                              ->root_vma()
                              ->create_subvma(1, kVmaFlags, "k:GDT", &gdtvma);
        if (Status::Ok != status) {
            log::trace("Failed to allocate VMA for GDT: {}", to_string(status));
        }

        x86_load_gdt();
    }

    auto x86_dump_gdt() -> void {
    }

} // namespace ours