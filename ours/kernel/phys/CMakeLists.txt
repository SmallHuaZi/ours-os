add_library(kernel_phys_headers INTERFACE)
add_library(kernel::phys::headers ALIAS kernel_phys_headers)
target_include_directories(kernel_phys_headers
INTERFACE
    "include"
)
target_link_libraries(kernel_phys_headers
INTERFACE
    kernel::main::headers
    kernel::phys::lib::bootmem::headers
)

add_library(kernel_phys_aspace INTERFACE)
target_sources(kernel_phys_aspace
INTERFACE
    "aspace.cpp"
)
target_link_libraries(kernel_phys_aspace
INTERFACE
    kernel_phys_headers
)

add_executable(kernel_phys)
add_executable(kernel::phys ALIAS kernel_phys)
target_sources(kernel_phys
PRIVATE
    "console.cpp"
    "panic.cpp"
    "kernel-package.cpp"
    "kernel-boot.cpp"
    "aspace.cpp"
    "phys-load.cpp"
    "memory.cpp"
)

if (NOT DEFINED OURS_CONFIG_ACPI)
    target_sources(kernel_phys
    PRIVATE "acpi.cpp" 
    )
endif()

target_compile_options(kernel_phys
PRIVATE
# "-v"
    "-g"
    "-O0"
    "-stdlib=libc++" # Help us to use headers from libcxx
    "-ffreestanding"
    # "-fPIC"
    "-fno-PIE" 
    "-fno-rtti"
    "-fno-builtin"
    # "-fno-leading-underscore"
    "-fno-exceptions"
    "-fno-stack-protector"
    "-fno-use-cxa-atexit"
    "-Wno-write-strings"
    "-mno-red-zone"
    "-mno-mmx" 
    "-mno-sse"
    "-fno-asynchronous-unwind-tables"
    "-fno-strict-aliasing"
)
target_compile_definitions(kernel_phys
PRIVATE
    ${OX_PREDEFINES}
)
target_link_libraries(kernel_phys
PRIVATE
    kernel::phys::headers
    kernel::phys::arch
    kernel::phys::lib::bootmem
    kernel::lib::libc
    kernel::lib::gktl::goi
    kernel::lib::omitl
    kernel::lib::acpi

    # Link to libgcc for some builtin routine, like __moddiv*, e.g..
    "libgcc.a"
)
target_link_options(kernel_phys
PRIVATE
# "-v"
    "-nostdlib"  # Prevent linker from linking against the c standard libraries 
    "-nostdlib++" # Prevent linker from linking against the c++ standard libraries
    "-nodefaultlibs"
    "-Wl,--no-pie"
    "-Wl,--no-relax"
    "-Wl,--build-id=none"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,--defsym,PHYS_LOAD_ADDRESS=${PHYS_LOAD_ADDRESS}"
    "-fno-register-global-dtors-with-atexit"
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/phys.ld"
    "-Wl,-z,max-page-size=${MAX_PAGE_SIZE}"
    "-static"

    # WARN This is a hosted dependency flag
    # "-L/usr/lib/gcc/x86_64-linux-gnu/13"
)

add_dependencies(kernel_phys kernel_main_image)

add_subdirectory("lib")
add_subdirectory("tests")