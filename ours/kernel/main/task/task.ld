SECTIONS {
   .init.rodata.schedulers: {
        *(g_eevdf_scheduler)
   } 
} INSERT AFTER .init.rodata