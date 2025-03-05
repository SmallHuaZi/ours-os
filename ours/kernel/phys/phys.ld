ENTRY(PHYS_ENTRY)

PHDRS {
    code    PT_LOAD  FLAGS(5);  /* R|X */
    rodata  PT_LOAD  FLAGS(4);  /* R */
    data    PT_LOAD  FLAGS(6);  /* R|W */
    note    PT_NOTE  FLAGS(0);
}

SECTIONS {
    . = 0x100000;
    
    .text : {
        *(.boot.header*)
        *(.text*)
    } :code

    .rodata.console : {
        PROVIDE_HIDDEN(__preset_console_start = .);
        KEEP(*(.rodata.console*))
        PROVIDE_HIDDEN(__preset_console_end = .);
    } :rodata

    .rodata : {
        *(.rodata*)
    }

    . = ALIGN(8);
    .init_array : {
        PROVIDE_HIDDEN(__ctors_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
               SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        PROVIDE_HIDDEN(__ctors_end  = .);

        /* ASSERT(ALIGNOF(.init_array) == 8 || SIZEOF(.init_array) == 0, ".init_array overalignment -> maybe padding gaps"); */
    }

    .image : {
        PROVIDE_HIDDEN(__image_start = .);
        *(.image)
        PROVIDE_HIDDEN(__image_end = .);
    }

    .data : {
        *(.data*)
    } :data

    . = ALIGN(8);
    .bss : {
        PROVIDE_HIDDEN(__bss_start = .);
        *(.bss*)
        PROVIDE_HIDDEN(__bss_end = .);
    }
}
