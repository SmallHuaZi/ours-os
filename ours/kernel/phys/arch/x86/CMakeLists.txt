add_library(x86_phys32_common)
target_sources(x86_phys32_common  
PRIVATE
    "start32.S"
)
target_link_libraries(x86_phys32_common
PRIVATE
    ours::kernel::common::headers
)
target_compile_options(x86_phys32_common
PRIVATE
    "-m32"
)

target_sources(ours_kernel_phys_arch
INTERFACE
    "image.S"
    "vga_logger.cpp"
)
target_include_directories(ours_kernel_phys_arch
INTERFACE
    ${CMAKE_BINARY_DIR}/gen
)

# If 32-bit CPU 
target_link_libraries(ours_kernel_phys_arch 
INTERFACE
    x86_phys32_common
    ours::kernel::lib::lz4
    ours::kernel::phys::boot
)

target_compile_options(ours_kernel_phys_arch
INTERFACE
    "-m32"
)

add_subdirectory("boot")
