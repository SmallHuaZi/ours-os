add_library(x86_phys_multiboot INTERFACE)
target_sources(x86_phys_multiboot 
INTERFACE
    "start.S"
    "legacy-boot.cpp"
)
target_link_libraries(x86_phys_multiboot
INTERFACE
    ours::kernel::phys::lib::multiboot
)

target_link_libraries(x86_phys_boot
INTERFACE
    phys_multiboot
)