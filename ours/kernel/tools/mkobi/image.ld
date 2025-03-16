ENTRY(MAIN_ENTRY)

PHDRS {
    image PT_LOAD FLAGS(7);     /* PF_R|PF_W|PF_X */
}

SECTIONS {
    . = 0;

    .image.header : {
        KEEP(*(.image.header))
    } :image

    .image.body :  {
        PROVIDE_HIDDEN(__image_start = .);
        KEEP(*(.image.body))
        PROVIDE_HIDDEN(__image_end = .);
    }
}
