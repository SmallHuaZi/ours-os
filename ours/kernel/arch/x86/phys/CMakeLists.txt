target_include_directories(kernel_phys_headers
INTERFACE
    "include"
)

#-------------------------------------
# Common code in phys 32 enviroment 
#-------------------------------------
add_library(x86_phys32 STATIC)
target_sources(x86_phys32
PRIVATE
    "start32.S"
)
target_link_libraries(x86_phys32
PRIVATE
    kernel::main::headers
    kernel::phys::headers
    kernel::lib::arch::headers
    cft::asmflags
)
target_link_options(x86_phys32
PRIVATE
    "-Wl,-m,elf_i386"
)

#-------------------------------------
# Common code in phys 64 enviroment 
#-------------------------------------
add_library(x86_phys64 STATIC)
target_sources(x86_phys64
PRIVATE
    "start64.S"
    # "paging64.S"
)
target_link_libraries(x86_phys64
PRIVATE
    kernel::main::headers
    kernel::phys::headers
    kernel::lib::arch::headers
    cft::asmflags
)

#-------------------------------------
# Common code for legacy bootloader 
#-------------------------------------
add_library(x86_legacy_boot INTERFACE)
target_sources(x86_legacy_boot
INTERFACE
    "legacy-boot.cpp"
)

#-------------------------------------
# Common code for multiboot 
#-------------------------------------
add_library(x86_legacy_multioot INTERFACE)
target_sources(x86_legacy_multioot 
INTERFACE
    "legacy-multiboot.cpp"
    "legacy-multiboot-start.S"
)
target_link_libraries(x86_legacy_multioot
INTERFACE
    cft::asmflags
)

#-------------------------------------
# Common console code for 32/64bit environment
#-------------------------------------
add_library(x86_phys_common INTERFACE)
target_sources(x86_phys_common
INTERFACE
    "gdt.S"
    "vga_console.cpp"
    "serial_console.cpp"
)
target_link_libraries(x86_phys_common
INTERFACE
    kernel::main::headers
    kernel::phys::headers
    kernel::lib::arch::headers
)

#------------------------------------
# Phys kernel target
#------------------------------------
add_library(kernel_phys_arch INTERFACE)
add_library(kernel::phys::arch ALIAS kernel_phys_arch)
target_link_libraries(kernel_phys_arch
INTERFACE
    kernel::main::headers
    kernel::phys::headers
    x86_boot_shim 
    x86_phys_common
    x86_legacy_boot 
    x86_legacy_multioot 
)
target_include_directories(kernel_phys_arch
INTERFACE
    ${CMAKE_BINARY_DIR}/gen
)

if(DEFINED kernel_phys32_env)
    target_link_libraries(kernel_phys_arch INTERFACE x86_phys32)
else()
    target_link_libraries(kernel_phys_arch INTERFACE x86_phys64)
endif(DEFINED kernel_phys32_env)


add_subdirectory("boot-shim")
