add_library(ours_kernel_arch_headers INTERFACE)
add_library(ours::kernel::arch::headers ALIAS ours_kernel_arch_headers)
target_include_directories(ours_kernel_arch_headers 
INTERFACE
    "include"
)

add_library(ours_kernel_arch INTERFACE)
add_library(ours::kernel::arch ALIAS ours_kernel_arch)
target_link_libraries(ours_kernel_arch
INTERFACE
    ours::kernel::arch::headers 
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") 
    add_subdirectory(x86)
else()
    add_subdirectory(${CMAKE_SYSTEM_PROCESSOR})
endif()