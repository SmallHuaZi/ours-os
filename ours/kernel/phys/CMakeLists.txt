add_subdirectory(lib)

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

add_executable(ours_kernel_phys "placeholder.cpp")
add_executable(ours::kernel::phys ALIAS ours_kernel_phys)
target_link_libraries(ours_kernel_phys
PUBLIC
    ours::kernel::phys::headers 
)