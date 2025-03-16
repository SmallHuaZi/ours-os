#include <ours/init.hpp>
#include <logz4/log.hpp>
#include <arch/x86/descriptor.hpp>

namespace ours {
    struct PACKED IdtEntry {
        u16 offset_low;   // 低 16 位偏移
        u16 selector;     // 代码段选择子
        u8  ist;          // IST 索引
        u8  type_attr;    // 类型和 DPL
        u16 offset_mid;   // 中间 16 位偏移
        u32 offset_high;  // 高 32 位偏移
        u32 zero;         // 保留
    };

    struct IdtEntry IDT[256];

    static auto set_idt_entry(u32 vec, void *handler) -> void {
        usize const addr = (usize)handler;
        IDT[vec].offset_low  = addr & 0xFFFF;
        IDT[vec].selector    = 0x08;
        IDT[vec].ist         = 0;
        IDT[vec].type_attr   = 0x8E;
        IDT[vec].offset_mid  = (addr >> 16) & 0xFFFF;
        IDT[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;
        IDT[vec].zero        = 0;
    }

    INIT_CODE
    static auto handle_early_irq(u32 vec) -> void {
        log::trace("Trigger interrupt {}\n", vec);
    }

    auto x86_setup_idt_early() -> void {
        for (auto i = 0; i < std::size(IDT); ++i) {
            set_idt_entry(i, reinterpret_cast<void *>(handle_early_irq));
        }
    }

    auto x86_setup_idt() -> void {

    }

} // namespace ours