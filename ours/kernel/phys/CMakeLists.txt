add_library(ours_kernel_phys_headers INTERFACE)
add_library(ours::kernel::phys::headers ALIAS ours_kernel_phys_headers)
target_include_directories(ours_kernel_phys_headers
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_phys_headers
INTERFACE
    ours::kernel::common::headers
    ours::kernel::phys::lib::bootmem::headers
)

add_executable(ours_kernel_phys)
add_executable(ours::kernel::phys ALIAS ours_kernel_phys)
target_link_libraries(ours_kernel_phys
PRIVATE
    ours::kernel::phys::headers
    ours::kernel::phys::arch
)

add_dependencies(ours_kernel_phys ours::kernel::main::image)

add_custom_command(
    TARGET ours_kernel_phys PRE_BUILD
    COMMAND echo "#define MAIN_MODULE_IMAGE ${CMAKE_BINARY_DIR}/ours_kernel_main_image" > arch/${OURS_ARCH}/main-module-image.hpp
)


add_subdirectory("lib")
add_subdirectory("arch")
