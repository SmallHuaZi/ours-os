SECTIONS {
    /* 
        The tests tagged |OURS_TEST| will be grouped into this
        section and executed conditionally.

        Here do not worry the usage of memory, because that it 
        has the same lifetime with other `init` sections.
     */
    .init.unittests : ALIGN(8) {
        PROVIDE_HIDDEN(__init_unittests_start = .);
        KEEP(*(SORT_BY_ALIGNMENT(.init.rodata.unittest.init_hook*)))
        PROVIDE_HIDDEN(__init_unittests_end = .);
    }
} INSERT BEFORE .init.rodata