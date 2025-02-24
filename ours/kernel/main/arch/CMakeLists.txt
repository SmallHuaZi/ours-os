add_library(ours_kernel_main_arch_header INTERFACE)
add_library(ours::kernel::main::arch::headers ALIAS ours_kernel_main_arch_header)
target_include_directories(ours_kernel_main_arch_header
INTERFACE
    "${OURS_ARCH}/include"
)

add_library(ours_kernel_main_arch INTERFACE)
add_library(ours::kernel::main::arch ALIAS ours_kernel_main_arch)
target_link_libraries(ours_kernel_main_arch
INTERFACE
    ours_kernel_main_arch_header
)

add_subdirectory(${OURS_ARCH})