add_library(kernel_platform_headers INTERFACE)
add_library(kernel::platform::headers ALIAS kernel_platform_headers)
target_include_directories(kernel_platform_headers 
INTERFACE
    "include"
)

add_library(kernel_platform INTERFACE)
add_library(kernel::platform ALIAS kernel_platform)
target_link_libraries(kernel_platform
INTERFACE
    "kernel::platform::headers"
)

if (CONFIG_PLATFORM_PC)
    message("[platform] add PC support code")
    add_subdirectory("pc")
    target_link_libraries(kernel_platform 
    INTERFACE
        "kernel::platform::pc"
    )
endif (CONFIG_PLATFORM_PC)