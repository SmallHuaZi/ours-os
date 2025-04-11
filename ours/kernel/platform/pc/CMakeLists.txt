add_library(kernel_platform_pc INTERFACE)
add_library(kernel::platform::pc ALIAS kernel_platform_pc)

set(platform_source_set 
    "init.cpp"
    "memory.cpp"
    "interrupt.cpp"
)
if (CONFIG_ACPI)
    list(APPEND platform_source_set "acpi.cpp")
    target_link_libraries(kernel_platform_pc 
    INTERFACE
        "kernel::lib::acpi"
    )
endif ()

target_sources(kernel_platform_pc INTERFACE ${platform_source_set})