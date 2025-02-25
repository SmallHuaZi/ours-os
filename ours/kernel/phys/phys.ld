// ours boot image

ENTRY(OBIMAGE_ENTRY)

SECTIONS {
    .text : {
        *(.boot.header)
        *(.text)
        *(.text.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    . = ALIGN(CONSTANT(PAGE_SIZE)
    PROVIDE_HIDDEN(__image_start = .);
    .image : {
        *(.image)
    }
    PROVIDE_HIDDEN(__image_end = .);

    . = ALIGN(8)
    PROVIDE_HIDDEN(__bss_start = .);
    .bss : {
        *(.bss)
        *(.bss.*)
    }
    PROVIDE_HIDDEN(__bss_end = .);
}