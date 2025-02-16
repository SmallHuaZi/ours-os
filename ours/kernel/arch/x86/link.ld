OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)
ENTRY(x86_start)
SECTIONS {
    . = 0x100000;

    KERNEL_START = .;
    KERNEL_CORE_START = .;
    .text : {
        KEEP(*(.multiboot))
        . = ALIGN(8);
        KERNEL_CODE_START = .;
        *(.text)
        KERNEL_CODE_END = .;
        . = ALIGN(4096);

        KERNEL_VDSO_START = .;
        *(.text.vdso)
        KERNEL_VDSO_END = .;
    }

    .data : {
        KERNEL_DATA_START = .;
        *(.data)
        . = ALIGN(4096);
        KERNEL_DATA_END = .;
    }

    KERNEL_BSS_START = .;
    .bss : {
        *(.bss)
        *(.bss.*)

        KERNEL_CPU_LOCAL_START = .;
        *(.kernel.cpu_local)
        KERNEL_CPU_LOCAL_END = .;

        KERNEL_COUNTER_START = .;
        *(.kernel.counter)
        KERNEL_COUNTER_END = .;
    }
    KERNEL_BSS_END = .;

    KERNEL_RODATA_START = .;
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    KERNEL_RODATA_END = .;
    KERNEL_CORE_END = .;

    KERNEL_EARLY_START = .;
    .early : {
        /* gktl::static_objects */
        CTORS_START = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        CTORS_END = .;
        
        KERNEL_INIT_CODE_START = .;
        *(.early.text)
        *(.early.text.*)
        KERNEL_INIT_CODE_END = .;

        /* gktl::init_hook */
        KERNEL_INIT_HOOK_START = .;
        *(.kernel.init_hook)
        *(.kernel.init_hook.*)
        KERNEL_INIT_HOOK_END = .;

        KERNEL_INIT_DATA_START = .;
        *(.early.data)
        *(.early.data.*)
        KERNEL_INIT_DATA_END = .;

        KERNEL_EARLY_RODATA_START = .;
        *(.early.rodata)
        *(.early.rodata.*)
        KERNEL_EARLY_RODATA_END = .;
    }
    KERNEL_EARLY_START = .;
    KERNEL_END = .;


    /DISCARD/ : { *(.fini_array*) *(.comment) }
}