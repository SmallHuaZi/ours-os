#include <ours/kernel_layout.hpp>

namespace ours {
    // Represent the layout of virtual memory from kernel
    EARLY_DATA_READONLY
    static KernelLayout const KERNEL_LAYOUT[] {
        { "Kernel text", KERNEL_TEXT_START, KERNEL_TEXT_END, MmuFlags::Readable | MmuFlags::Present },
        { "Kernel data", KERNEL_DATA_START, KERNEL_DATA_END, MmuFlags::Present },
        { "Kernel bss", KERNEL_BSS_START, KERNEL_BSS_END, MmuFlags::Present },
        { "Kernel early text", KERNEL_EARLY_TEXT_START, KERNEL_EARLY_TEXT_END, MmuFlags::Present },
        { "Kernel early data", KERNEL_EARLY_DATA_START, KERNEL_EARLY_DATA_END, MmuFlags::Present },

        // Frame map
        { "Frame map", FRAME_MAP_VIRT_ADDR_START, FRAME_MAP_VIRT_ADDR_END, MmuFlags::Present },
    };
}