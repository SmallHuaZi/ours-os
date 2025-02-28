// These symbols were defined in main.ld
#include "ours_kernel_main_raw_image.hpp"
#include <ours/asm.hpp>
#include <omi/asm.hpp>

.extern MAIN_ENTRY
.extern MI_END
.extern MI_CORE_SIZE

.section .image, "a", @progbits

#ifdef CHECK_IMAGE
    .byte 'O','U','R','S','-','K','E','R','N','E','L'
#endif

// NAME, END, FLAGS
OMI_HEADER(FILE_INFO, MI_END - FILE_INFO)
OMI_HEADER(MAIN_INFO, MI_CORE_SIZE)

DATA(OMI_KERNEL_PAYLOAD)
    // Entry point
    SIZE(MAIN_ENTRY)
    // Scratch memory for arch reserved content
    SIZE(0)
DATA_END(OMI_KERNEL_PAYLOAD)

# Includes the main image compressed
.incbin IMAGE_FILE

// If we need fixup codes, put it into the bottom.