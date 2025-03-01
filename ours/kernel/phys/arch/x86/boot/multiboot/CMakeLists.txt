add_library(x86_phys_multiboot INTERFACE)
target_sources(x86_phys_multiboot 
INTERFACE
    "start.S"
    "legacy-boot.cpp"
)

target_link_libraries(x86_phys_boot
INTERFACE
    x86_phys_multiboot
)

target_link_options(x86_phys_boot
INTERFACE
    "-Wl,-m,elf_i386"
)