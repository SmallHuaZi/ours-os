add_library(ours_kernel_common_headers INTERFACE)
add_library(ours::kernel::common::headers ALIAS ours_kernel_common_headers)
target_include_directories(ours_kernel_common_headers 
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_common_headers
INTERFACE
    ours::system::public::headers
    ours::kernel::lib::ktl::headers
    ours::kernel::lib::gktl::headers
    ours::kernel::lib::ustl::headers
)

add_subdirectory("arch")

add_subdirectory("core")

add_subdirectory("lib")

add_subdirectory("object")

add_subdirectory("phys")

add_executable(ours_kernel)
add_executable(ours::kernel ALIAS ours_kernel)
target_link_libraries(ours_kernel
PRIVATE
    ours::kernel::arch
    ours::kernel::core
)