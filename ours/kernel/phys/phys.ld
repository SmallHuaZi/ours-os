ENTRY(PHYS_ENTRY)

PHDRS {
    code    PT_LOAD  FLAGS(5);  /* R|X */
    rodata  PT_LOAD  FLAGS(4);  /* R */
    data    PT_LOAD  FLAGS(6);  /* R|W */
    note    PT_NOTE  FLAGS(0);
}

SECTIONS {
    . = PHYS_LOAD_ADDRESS;
    PROVIDE_HIDDEN(__executable_start = .);
    . = ALIGN(8);
    .boot : {
        *(.boot.header*)
        *(.boot.text*)
    } :code
    
    . = ALIGN(8);
    .text : {
        *(.text*)
    }
    
    /*
     * PLT entry sections.
     *
     * The compiler and linker should never generate PLT entries since all
     * symbols have hidden visibility.
     */
    .plt : { *(.plt) }
    ASSERT(SIZEOF(.plt) == 0, "unexpected '.plt' entries")
    .plt.got : { *(.plt.got) }
    ASSERT(SIZEOF(.plt.got) == 0, "unexpected '.plt.got' entries")

    . = ALIGN(8);
    .rodata : {
        *(.rodata*)
    } :rodata

    . = ALIGN(8);
    .init_array : {
        PROVIDE_HIDDEN(__ctors_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN(__ctors_end  = .);

        ASSERT(ALIGNOF(.init_array) == 8 || SIZEOF(.init_array) == 0, ".init_array overalignment -> maybe padding gaps"); */
    }

    .data : {
        . = ALIGN(8);
        *(.data*)
    } :data

    . = ALIGN(8);
    .bss : {
        PROVIDE_HIDDEN(__bss_start = .);
        *(.sbss*) *(.bss* COMMON)
        PROVIDE_HIDDEN(__bss_end = .);
    }
    PROVIDE_HIDDEN(__executable_end = .);
}
