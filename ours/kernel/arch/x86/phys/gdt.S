#include <ours/arch/asm.hpp>
#include <ours/macro_abi.hpp>
#include <ours/mem/cfg.hpp>

/// Common to 32/64 bits platform.
.object g_temp_gdtr, type=data, scope=global, align=8
    .short g_early_gdt_end - g_early_gdt - 1
    .quad g_early_gdt
.end_object

/// GDT
.object g_early_gdt, type=data, scope=global, align=8
    .quad 0                   # NULL
    .quad 0x00cf9a000000ffff  # kernel code32 (0x08)
    .quad 0x00af9a000000ffff  # kernel code64 (0x10)
    .quad 0x00cf92000000ffff  # kernel data (0x18)
.label g_early_gdt_end, global
.end_object

.object g_temp_idtr, type=data, scope=global, align=8
	.word g_early_idt_end - g_early_idt - 1
	.quad 0
.end_object

	.balign 8
.object g_early_idt, type=data, scope=global, align=8
	.rept EALY_IDT_ENTRIES
	.quad 0
	.quad 0
	.endr
.label g_early_idt_end, global
.end_object

/// Executable stack
.object g_early_stack, type=bss, scope=global, align=8
    .skip (PAGE_SIZE << 2) // 16K
.label g_early_stack_end, global
.end_object