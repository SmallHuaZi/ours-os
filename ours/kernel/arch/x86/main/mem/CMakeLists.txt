set(source_set 
    "mmu.cpp"
    "fault.cpp"
    "vm_aspace.cpp"
    "page_table.cpp"
)

if (CONFIG_NUMA)
    list(APPEND source_set "numa.cpp")
endif ()

if (CONFIG_ACPI)
    list(APPEND source_set "srat.cpp")
endif ()

target_sources(kernel_main_arch INTERFACE ${source_set})