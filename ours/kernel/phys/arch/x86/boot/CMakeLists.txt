add_library(x86_phys_boot INTERFACE)
add_library(ours::kernel::phys::boot ALIAS x86_phys_boot)
target_sources(x86_phys_boot
INTERFACE
    "shim.cpp"
)

add_subdirectory("multiboot")