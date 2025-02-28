ENTRY(PHYS_ENTRY)

SECTIONS {
    .text : {
        *(.boot.header)
        *(.text*)
    } :code

    .rodata : {
        *(.data*)
    } :rodata

    . = ALIGN(CONSTANT(PAGE_SIZE)
    .image : {
        PROVIDE_HIDDEN(__image_start = .);
        *(.image)
        PROVIDE_HIDDEN(__image_end = .);
    }

    .data : {
        *(.data*)
    } :data

    . = ALIGN(8)
    .bss : {
        PROVIDE_HIDDEN(__bss_start = .);
        *(.bss*)
        PROVIDE_HIDDEN(__bss_end = .);
    }
}

PHDRS {
    code    PT_LOAD  FLAGS(4);  /* R */
    rodata  PT_LOAD  FLAGS(4);  /* R */
    data    PT_LOAD  FLAGS(6);  /* R|W */
    note    PT_NOTE  FLAGS(0);
}