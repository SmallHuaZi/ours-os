ENTRY(MAIN_ENTRY)

SECTIONS {
    . = IMAGE_LOAD_START;

    .image : {
        KEEP(*(.image))

        /*
         * This includes the fixup code, which overlaps the bss.
         */
        IMAGE_LOAD_END = .;

        /*
         * This is the end of the kernel load image proper, where the bss
         * starts at runtime and the fixup code starts in the image.
         */
        ASSERT(ABSOLUTE(IMAGE_LOAD_KERNEL_END) < ABSOLUTE(IMAGE_LOAD_END),
               "image symbols bad");
    } :image

    /*
     * This is the high bound of the address range that must be reserved.
     * Since the fixups and the bss overlap and are both of varying sizes,
     * this might be past IMAGE_LOAD_END (more bss than fixups) or vice versa.
     */
    IMAGE_RESERVE_END = IMAGE_MEMORY_END + IMAGE_RESERVE_SIZE;

    /*
     * When a boot loader is actually using the ELF headers, it needs to
     * know how much memory to reserve after the load image (p_filesz is
     * the load image, and p_memsz > p_filesz to indicate the extra space
     * to reserve).  This ensures that the segment has the right p_memsz.
     */
    .bss : {
        . = MAX(ABSOLUTE(IMAGE_RESERVE_END), ABSOLUTE(.));
        ASSERT(ABSOLUTE(.) >= ABSOLUTE(IMAGE_RESERVE_END), "image layout bad");
    }
}

PHDRS {
    image PT_LOAD FLAGS(7);     /* PF_R|PF_W|PF_X */
}