#include "main-module-image.hpp"

.org BOOT_HEADER_SIZE

.section .image, "a", @progbit
.incbin "x86_raw_main_image", BOOT_HEADER_SIZE