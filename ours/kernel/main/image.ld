/**
 *  The range [MI_LOAD_START, MI_LOAD_END] give out the real layout of main module in virutal memory.
 *
 */
ENTRY(MAIN_ENTRY)

SECTIONS {
    . = MAIN_BASE;

    /* 主模块镜像和架构保留的一部分内容都放在.image段中 */
    .image :  {
        PROVIDE_HIDDEN(__image_start = .);
        KEEP(*(.image))
        PROVIDE_HIDDEN(__image_end = .);
    } :image

    MI_END = .;
}

PHDRS {
    image PT_LOAD FLAGS(7);     /* PF_R|PF_W|PF_X */
}