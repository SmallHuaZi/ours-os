add_library(x86_boot_shim INTERFACE)
target_sources(x86_boot_shim 
INTERFACE
    "shim.cpp"
)