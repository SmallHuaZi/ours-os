#include <arch/x86/descriptor.hpp>
#include <ours/init.hpp>
#include <ours/cpu_local.hpp>

using namespace arch;

namespace ours {
    typedef Gdt32                   Gdt;
    typedef Gdt::Entry              Gdte;

    static constexpr usize const GDT_SIZE = 32;

    alignas(8) Gdte S_GDT_SELECTORS[GDT_SIZE] = {
        /* [0] */ {0, 0x00000, 0},
        /* [Gdt::KERNEL_CODE64] */ {0, 0xfffff, GdteFlags::KernelCode64},
        /* [Gdt::KERNEL_CODE32] */ {0, 0xfffff, GdteFlags::KernelCode32},    
        /* [Gdt::KERNEL_DATA] */ {0, 0xfffff, GdteFlags::KernelData},
        /* [Gdt::USER_CODE64] */ {0, 0xfffff, GdteFlags::UserCode64},
        /* [Gdt::USER_CODE32] */ {0, 0xfffff, GdteFlags::UserCode32},    
        /* [Gdt::USER_DATA] */ {0, 0xfffff, GdteFlags::UserData},
    };

    // Gdt pointer.
    extern "C" Gdt G_GDT { S_GDT_SELECTORS, sizeof(S_GDT_SELECTORS) };

    extern "C" INIT_CODE auto setup_gdt() -> void
    {
        // auto x = Gdt::current();
        // Log::debug("Before loading, Current GDT {%d, %p}\n", x.length, x.table);

        // Load GDT
        G_GDT.load();

        // x = Gdt::current();
        // Log::debug("After loading, Current GDT {%d, %p}\n", x.length, x.table);
        // for (auto i = 0; i < 7; ++i) {
        //     auto entry = x.get_entry(i);
        //     Log::debug("Entry {%x, %x}\n", entry.base(), entry.limit());
        // }
    }
}