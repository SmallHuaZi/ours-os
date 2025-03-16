ENTRY(MAIN_ENTRY)

PHDRS {
    code    PT_LOAD  FLAGS(5);  /* R|X */
    init    PT_LOAD  FLAGS(7);  /* R|W|X */
    rodata  PT_LOAD  FLAGS(4);  /* R */
    data    PT_LOAD  FLAGS(6);  /* R|W */
    note    PT_NOTE  FLAGS(0);
}

SECTIONS {
    . = 0;

    PROVIDE_HIDDEN(__executable_start = .);

    /* Will be reclaimed after kernel initialization. */
    PROVIDE_HIDDEN(__init_start = .);

    .init.text : AT(ADDR(.init.text)) {
        PROVIDE_HIDDEN(__init_code_start = .);
        *(SORT_BY_ALIGNMENT(.init.boot*))
        *(SORT_BY_ALIGNMENT(.init.text*))
        PROVIDE_HIDDEN(__init_code_end = .);
    } :init

    .init.data : AT(ADDR(.init.data)) {
        PROVIDE_HIDDEN(__init_data_start = .);
        *(SORT_BY_ALIGNMENT(.init.data*))
        PROVIDE_HIDDEN(__init_data_end = .);
    }

    .init.rodata : AT(ADDR(.init.rodata)) {
        PROVIDE_HIDDEN(__init_rodata_start = .);
        *(SORT_BY_ALIGNMENT(.init.rodata*))
        PROVIDE_HIDDEN(__init_rodata_end = .);
    }

    .kernel.cpu_local : {
        PROVIDE_HIDDEN(__cpu_local_start = .);
        *(SORT_BY_ALIGNMENT(.kernel.cpu_local*))
        PROVIDE_HIDDEN(__cpu_local_end = .);
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

    .init.init_hook : {
        /* gktl::init_hook */
        KERNEL_INIT_HOOK_START = .;
        *(SORT_BY_ALIGNMENT(.init.init_hook*))
        KERNEL_INIT_HOOK_END = .;
    }
    PROVIDE_HIDDEN(__init_end  = .);
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    PROVIDE_HIDDEN(__code_start = .);
    .text : {
        *(SORT_BY_ALIGNMENT(.text*))
    } :code
    PROVIDE_HIDDEN(__code_end = .);
    /* To support the huge page mapping. */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    PROVIDE_HIDDEN(__rodata_start = .);
    .interp : { *(.interp) }
    .dynamic : { *(.dynamic) }
    .hash : { *(.hash) }
    .gnu.hash : { *(.gnu.hash) }
    .dynsym : { *(.dynsym) }
    .dynstr : { *(.dynstr) }
    .relr.auth.dyn : { *(.relr.auth.dyn) }
    ASSERT(SIZEOF(.relr.auth.dyn) == 0, ".relr.auth.dyn is not supported")
    .relr.dyn : { *(.relr.dyn) }

    .rodata : {
        *(SORT_BY_ALIGNMENT(.rodata*))
    } :rodata
    
    .got : { *(.got*) }

    .note.gnu.build-id : {
        PROVIDE_HIDDEN(__build_id_note_start = .);
        *(.note.gnu.build-id)
        PROVIDE_HIDDEN(__build_id_note_end = .);
        /*
         * Record the build ID size, without the note header (including name)
         * of 16 bytes.  This is used below.
         */
        HIDDEN(__build_id_size = ABSOLUTE(__build_id_note_end - __build_id_note_start) - 16);
    } :rodata :note
    PROVIDE_HIDDEN(__rodata_end = .);

    PROVIDE_HIDDEN(__data_start = .);
    .data : {
        *(SORT_BY_ALIGNMENT(.data*))
    } :data
    PROVIDE_HIDDEN(__data_end = .);

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .bss : {
        PROVIDE_HIDDEN(__bss_start = .);
        *(SORT_BY_ALIGNMENT(.bss*))

        . = ALIGN(16);
        PROVIDE_HIDDEN(__counter_start = .);
        *(.kernel.counter)
        PROVIDE_HIDDEN(__counter_end = .);

        PROVIDE_HIDDEN(__bss_end = .);
    }

    PROVIDE_HIDDEN(__executable_end = .);

    ASSERT(__init_start == __executable_start, "`.init*` sections should be put on the front of kernel iamge")
    ASSERT(__bss_end == __executable_end, "`.bss*` sections should be put on the tail of kernel iamge")

    .got.plt (INFO) : { *(.got.plt) }
    ASSERT(SIZEOF(.got.plt) == 0 || SIZEOF(.got.plt) == 0x18, "Unexpected GOT/PLT entries detected!")
    .got : {
        *(.got) *(.igot.*)
    }
    ASSERT(SIZEOF(.got) == 0, "Unexpected GOT entries detected!")
    .plt : {
        *(.plt) *(.plt.*) *(.iplt)
    }
    ASSERT(SIZEOF(.plt) == 0, "Unexpected run-time procedure linkages detected!")
    .rel.dyn : {
        *(.rel.*) *(.rel_*)
    }
    ASSERT(SIZEOF(.rel.dyn) == 0, "Unexpected run-time relocations (.rel) detected!")
    .rela.dyn : {
        *(.rela.*) *(.rela_*)
    }
    ASSERT(SIZEOF(.rela.dyn) == 0, "Unexpected run-time relocations (.rela) detected!")

    /* Non-allocated section needs to be protected from GC with BFD ld. */
    .code-patches 0 : {
        KEEP(*(.code-patches))
    }
}

/* 
 * These symbols export to image.S by linker's parameters, '-Wl, --just-symbols'.
 *
 * Symbols named with prefix `MI_LOAD_` indicate the load address range bounds of main 
 * module. Among these symbols, MI_CORE_START give out the almost entirely required 
 * image content but the architecture specific image content.  
 *
 * And for storaging architecture specific into the final image, the MI_LOAD_END was defined 
 * in image.ld.
 *
 * MI_* indicates the bounds of kernel code address range.
*/
MAIN_LOAD_END = __data_end;
MAIN_RESERVED_SIZE = __executable_end - __data_end;