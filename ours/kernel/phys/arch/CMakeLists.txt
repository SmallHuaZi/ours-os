target_include_directories(ours_kernel_phys_headers
INTERFACE
    "${OURS_ARCH}/include"
)

add_library(ours_kernel_phys_arch INTERFACE)
add_library(ours::kernel::phys::arch ALIAS ours_kernel_phys_arch)
target_link_libraries(ours_kernel_phys_arch
INTERFACE
    ours::kernel::phys::headers
)

add_subdirectory(${OURS_ARCH})