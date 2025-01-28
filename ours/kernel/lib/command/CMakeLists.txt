add_library(ours_kernel_lib_command INTERFACE)
add_library(ours::kernel::lib::command ALIAS ours_kernel_lib_command)

target_include_directories(ours_kernel_lib_command 
    INTERFACE include
)

target_link_libraries(ours_kernel_lib_command 
    INTERFACE ours::kernel::lib::config 
)