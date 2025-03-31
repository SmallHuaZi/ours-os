/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef OMI_ASM_HPP
#define OMI_ASM_HPP 1

#include <omitl/macro.hpp>

/// Define a header object.
.macro .omi.header.object name, type, len, flags=0, extra=0, align=OMI_ALIGNMENT
    .object \name, rodata, align=\align, nosection=nosection
        .int    \type
        .int    \len
        .int    \flags
        .int    OMI_HEADER_MAGIC
        .int    0
        .int    \extra
    .end_object
.endm

/// Define a header object which describes whole obi information.
.macro .omi.contaienr.object name, len, flags=0, extra=0, align=OMI_ALIGNMENT
    .omi.header.object \name, OMIT_CONTAINER, \len, \flags, \extra, \align
.endm

/// Define a header object which describes kernel information.
.macro .omi.kernel.object name, len, flags=0, extra=0, align=OMI_ALIGNMENT
    .omi.header.object \name, OMIT_KPACKAGE, \len, \flags, \extra, \align
.endm

.macro .omi.kernel.image.object name, image_end, zipped=0, zipped_size=0
    .omi.contaienr.object  \name, len=(\image_end - .L.omi.kernel.object.\name\().\@)
    .if !\zipped
        .omi.kernel.object .L.omi.kernel.object.\name\().\@, len=(\image_end - .L.omi.kernel.image.object.payload.\name\().\@)
    .else
        .omi.kernel.object .L.omi.kernel.object.\name\().\@, len=(\image_end - .L.omi.kernel.image.object.payload.\name\().\@), flags=OMIF_ZIPPED, extra=\zipped_size
    .endif
    .label .L.omi.kernel.image.object.payload.\name\().\@, type=object
.endm

.macro .omi.kernel.header.object entry_point, reserved_size
    .quad \entry_point
    .quad \reserved_size
.endm

#endif // #ifndef OMI_ASM_HPP