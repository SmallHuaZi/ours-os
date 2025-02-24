add_library(x86_phys32_common INTERFACE)
target_sources(x86_phys32_common  
INTERFACE
    "start32.S"
)

target_sources(ours_kernel_phys_arch
INTERFACE
    "image.S"
    "early_console.cpp"
)

# If 32-bit CPU 
target_link_libraries(ours_kernel_phys_arch 
INTERFACE
    phys32_common
    ours::kernel::lib::lz4
)

add_subdirectory("boot")