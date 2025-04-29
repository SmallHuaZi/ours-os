#include <ours/arch/x86/idt.hpp>
#include <ours/arch/x86/faults.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/init.hpp>

#include <logz4/log.hpp>
#include <arch/x86/descriptor.hpp>

namespace ours {
    struct PACKED IdtEntry {
        auto set(usize handler) -> void {
            offset_low  = handler & 0xFFFF;
            selector    = X86_GDT_KERNEL_CODE64;
            ist         = 0;
            type_attr   = 0x8F; // Present, DPL=0 and INT gate
            offset_mid  = (handler >> 16) & 0xFFFF;
            offset_high = (handler >> 32) & 0xFFFFFFFF;
            zero        = 0;
        }

        u16 offset_low;
        u16 selector;
        u8  ist;
        u8  type_attr;
        u16 offset_mid;
        u32 offset_high;
        u32 zero;
    };

    struct Idt {
        auto load() const -> void {
            struct PACKED {
                u16 len;
                usize addr;
            } desc { sizeof(*this) - 1, reinterpret_cast<usize>(this) };
            asm volatile("lidt %0" :: "m"(desc));
        }

        auto begin() {
            return std::begin(entries_);
        }

        auto end() {
            return std::end(entries_);
        }

        IdtEntry entries_[256];
    };

    FORCE_INLINE
    static auto rectify_idt(IdtEntry &entry) -> void {
        entry.set(*reinterpret_cast<usize *>(&entry));
    }

    /// Defined in arch/x86/main/idt.S, default value per entry is 
    /// the hanler.
    NO_MANGLE Idt g_idt;
    static Idt *s_pidt = &g_idt;

    auto x86_load_idt() -> void {
        s_pidt->load();
    }

    auto x86_init_idt_early() -> void {
        for (auto &entry: g_idt) {
            rectify_idt(entry);
        }

        x86_dump_idt();
    }

    auto x86_setup_idt() -> void {
        // Remap IDT to a readonly page
        x86_load_idt(); 
    }

    auto x86_dump_idt() -> void {
        log::info("IDT Dumpping Infomation");
        for (auto i = 0; i < std::size(g_idt.entries_); ++i) {
            auto &entry = g_idt.entries_[i];
            log::info("  [0]=(handler=0x{:X}, DPL={}, TYPE={})", 
                entry.offset_low | (entry.offset_mid << 16) | (u64(entry.offset_high) << 32),
                (entry.type_attr >> 5) & 0x3,
                (entry.type_attr & 0xF) == 0xE ? "INTR" : "TRAP"
            );
        }
    }

} // namespace ours