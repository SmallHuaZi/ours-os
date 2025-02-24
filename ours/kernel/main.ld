ENTRY(MAIN_ENTRY)

SECTIONS {
    . = MAIN_LOAD_OFFSET;

    PROVIDE_HIDDEN(__executable_start = .);

    PROVIDE_HIDDEN(__code_start = .);
    .text : {
        *(.text)
        *(.text.*)
    }
    PROVIDE_HIDDEN(__code_end = .);
    // To support the huge page mapping.
    . = ALIGN(CONSTANT(MAX_PAGE_SIZE));

    PROVIDE_HIDDEN(__data_start = .);
    .data : {
        *(.data)
        *(.data.*)
    }
    PROVIDE_HIDDEN(__data_end = .);
    . = ALIGN(CONSTANT(MAX_PAGE_SIZE));

    PROVIDE_HIDDEN(__rodata_start = .);
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    PROVIDE_HIDDEN(__rodata_end = .);

    // Will be reclaimed after kernel initialization.
    PROVIDE_HIDDEN(__init_start = .);
    .init_array : {
        PROVIDE_HIDDEN(__ctors_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
               SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        PROVIDE_HIDDEN(__ctors_end  = .);

        ASSERT(ALIGNOF(.init_array) == 8 || SIZEOF(.init_array) == 0,
               ".init_array overalignment -> maybe padding gaps");
    }

    .init.text : {
        PROVIDE_HIDDEN(__init_code_start = .);
        *(.init.text)
        *(.init.text.*)
        PROVIDE_HIDDEN(__init_code_end = .);
    }

    .init.data : {
        PROVIDE_HIDDEN(__init_data_start = .);
        *(.init.data)
        *(.init.data.*)
        PROVIDE_HIDDEN(__init_data_end = .);
    }

    .init.rodata : {
        PROVIDE_HIDDEN(__init_rodata_start = .);
        *(.init.rodata)
        *(.init.rodata.*)
        PROVIDE_HIDDEN(__init_rodata_end = .);
    }

    .init.init_hook : {
        // gktl::init_hook
        KERNEL_INIT_HOOK_START = .;
        *(.init.init_hook)
        *(.init.init_hook.*)
        KERNEL_INIT_HOOK_END = .;
    }
    PROVIDE_HIDDEN(__init_end  = .);

    PROVIDE_HIDDEN(__bss_start = .);
    .bss : ALIGN(16) {
        *(.bss)
        *(.bss.*)

        PROVIDE_HIDDEN(__cpu_local_start = .);
        *(.kernel.cpu_local)
        PROVIDE_HIDDEN(__cpu_local_end = .);

        PROVIDE_HIDDEN(__counter_start = .);
        *(.kernel.counter)
        PROVIDE_HIDDEN(__counter_end = .);

        . = ALIGN(16)
    }
    PROVIDE_HIDDEN(__bss_end = .);

    PROVIDE_HIDDEN(__executable_end = .);

    ASSERT(__bss_end == __executable_end, "`init.*` sections should be put on the tail of kernel iamge")

    // Non-allocated section needs to be protected from GC with BFD ld.
    .code-patches 0 : {
        KEEP(*(.code-patches))
    }
}

MAIN_LOAD_START = __executable_start
MAIN_LOAD_END  = __executable_end