# Define a kernel object. 
function(ours_kernel_module module_name)
    add_library(ours_kernel_module_${module_name} OBJECT)
    add_library(ours::kernel::${module_name} ALIAS ours_kernel_module_${module_name})
endfunction(ours_kernel_module module_name)

# function(source_set module_name)
#     cmake_parse_arguments(
#         MODULE_NAME
#     )
#     target_sources(ours_kernel_module_${module_name}
#     ) 
# endfunction(source_set module_name)