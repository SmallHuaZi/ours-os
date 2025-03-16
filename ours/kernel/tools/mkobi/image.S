#include <ours/arch/asm.hpp>
#include <omi/asm.hpp>

.extern MAIN_RESERVED_SIZE
.extern MAIN_LOAD_END
.extern MAIN_ENTRY

// Header
.section .image.header, "a", %progbits
.omi.kernel.image.object    kernel_image, MAIN_LOAD_END, 1, (.Limage.body.end - .Limage.body.start)
.omi.kernel.header.object   MAIN_ENTRY, MAIN_RESERVED_SIZE

// TODO(SmallHuaZi) This is a wordaround and will be removed in the near future.
.section .image.body, "a", %progbits
# Includes the main image compressed
.label .Limage.body.start
// IMAGE_FILE is a macro provided by CMake
.incbin IMAGE_FILE
.label .Limage.body.end
