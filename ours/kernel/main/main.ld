ENTRY(MAIN_ENTRY)

SECTIONS {
    /* FIXME(SmallHuaZi) Now PIE kernel has not been supported, using a default address during the transition */
    . = 0;

    PROVIDE_HIDDEN(__executable_start = .);

    .interp : { *(.interp) }
    .dynamic : { *(.dynamic) }
    .hash : { *(.hash) }
    .gnu.hash : { *(.gnu.hash) }
    .dynsym : { *(.dynsym) }
    .dynstr : { *(.dynstr) }
    .relr.auth.dyn : { *(.relr.auth.dyn) }
    ASSERT(SIZEOF(.relr.auth.dyn) == 0, ".relr.auth.dyn is not supported")
    .relr.dyn : { *(.relr.dyn) }

    .note.gnu.build-id : {
        PROVIDE_HIDDEN(__build_id_note_start = .);
        *(.note.gnu.build-id)
        PROVIDE_HIDDEN(__build_id_note_end = .);
        /*
         * Record the build ID size, without the note header (including name)
         * of 16 bytes.  This is used below.
         */
        HIDDEN(__build_id_size = ABSOLUTE(__build_id_note_end - __build_id_note_start) - 16);
    }

    PROVIDE_HIDDEN(__code_start = .);
    .text : {
        *(SORT_BY_ALIGNMENT(.text*))
    }
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PROVIDE_HIDDEN(__code_end = .);

    PROVIDE_HIDDEN(__rodata_start = .);
    .rodata : {
        *(SORT_BY_ALIGNMENT(.rodata*))
    }

    PROVIDE_HIDDEN(__rodata_end = .);

    PROVIDE_HIDDEN(__data_start = .);
    .data : {
        *(SORT_BY_ALIGNMENT(.data*))
    }
    /* Aligned to Page Size */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PROVIDE_HIDDEN(__data_end = .);

    /*
     * LLD only makes particular named sections part of PT_GNU_RELRO, so
     * all the special RELRO sections have to be merged into one output section.
     */
    PROVIDE_HIDDEN(__relro_start = .);
    .got : { *(.got*) }
    .data.rel.ro : ALIGN(8) {
        PROVIDE_HIDDEN(__commands_start = .);
        *(.data.rel.ro.commands*)
        PROVIDE_HIDDEN(__commands_end = .);

        /* TODO(https://fxbug.dev/384994543): Alignment should actually be 8 if present. */
        *(.data.rel.ro*)
        ASSERT(ALIGNOF(.data.rel.ro) <= 32 || SIZEOF(.data.rel.ro) == 0,
                       ".data.rel.ro overalignment -> padding gaps");
    }
    /* Aligned to Page Size */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PROVIDE_HIDDEN(__relro_end = .);

    /* Will be reclaimed after kernel initialization. */
    PROVIDE_HIDDEN(__init_start = .);
    /* ASSERT(__executable_start  == __init_start, "`init` segment should be put on the front of kernel image") */

    PROVIDE_HIDDEN(__init_code_start = .);
    .init.text : AT(ADDR(.init.text)) {
        *(SORT_BY_ALIGNMENT(.init.boot*))
        *(SORT_BY_ALIGNMENT(.init.text*))
    }
    PROVIDE_HIDDEN(__init_code_end = .);

    PROVIDE_HIDDEN(__init_data_start = .);
    .init.data : AT(ADDR(.init.data)) {
        /* ours::CpuLocal */
        PROVIDE_HIDDEN(__cpu_local_start = .);
        *(SORT_BY_ALIGNMENT(.init.data.cpu_local*))
        PROVIDE_HIDDEN(__cpu_local_end = .);

        *(SORT_BY_ALIGNMENT(.init.data*))
    }
    PROVIDE_HIDDEN(__init_data_end = .);

    PROVIDE_HIDDEN(__init_rodata_start = .);
    .init.rodata : AT(ADDR(.init.rodata)) {
        /* gktl::InitHook */
        PROVIDE_HIDDEN(__init_hook_start = .);
        KEEP(*(SORT_BY_ALIGNMENT(.init.rodata.init_hook*)))
        PROVIDE_HIDDEN(__init_hook_end = .);

        *(SORT_BY_ALIGNMENT(.init.rodata*))
    }

    .init_array : {
        PROVIDE_HIDDEN(__ctors_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
               SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        PROVIDE_HIDDEN(__ctors_end  = .);

        ASSERT(ALIGNOF(.init_array) == 8 || SIZEOF(.init_array) == 0,
               ".init_array overalignment -> maybe padding gaps");
    }
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PROVIDE_HIDDEN(__init_rodata_end = .);
    PROVIDE_HIDDEN(__init_end  = .);

    PROVIDE_HIDDEN(__bss_start = .);
    .bss : ALIGN(16) {
        *(SORT_BY_ALIGNMENT(.bss*))

        . = ALIGN(16);
        PROVIDE_HIDDEN(__counter_start = .);
        *(.kernel.counter)
        PROVIDE_HIDDEN(__counter_end = .);

        /* Aligned to Page Size */
        . = ALIGN(CONSTANT(MAXPAGESIZE));
    }
    PROVIDE_HIDDEN(__bss_end = .);

    PROVIDE_HIDDEN(__executable_end = .);

    ASSERT(__bss_end == __executable_end, "`.bss*` sections should be put on the tail of kernel image")

    /* Non-allocated section needs to be protected from GC with BFD ld. */
    .code-patches 0 : {
        KEEP(*(.code-patches))
    }
}
