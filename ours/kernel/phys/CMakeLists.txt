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

add_executable(kernel_phys)
add_executable(kernel::phys ALIAS kernel_phys)
target_sources(kernel_phys
PRIVATE
    "acpi.cpp"
    "console.cpp"
    "loader.cpp"
    "panic.cpp"
)

target_compile_options(kernel_phys
PRIVATE
# "-v"
    "-g"
    "-O0"
    "-stdlib=libc++"
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
target_link_libraries(kernel_phys
PRIVATE
    kernel::phys::headers
    kernel::phys::arch
    kernel::phys::lib::bootmem
    kernel::lib::libc
    kernel::lib::gktl::goi
    kernel::lib::omi
    kernel::lib::acpi
    "libgcc.a"
)
target_link_options(kernel_phys
PRIVATE
# "-v"
    # Link to libgcc for some builtin routine.
    "-nostdlib"
    "-nostdlib++"
    "-nodefaultlibs"
    "-Wl,--no-pie"
    "-Wl,--no-relax"
    "-Wl,--build-id=none"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,--defsym,PHYS_LOAD_ADDRESS=${PHYS_LOAD_ADDRESS}"
    "-fno-register-global-dtors-with-atexit"
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/phys.ld"
    "-static"

    # WARN This is a hosted dependency flag
    "-L/usr/lib/gcc/x86_64-linux-gnu/13"
)

add_dependencies(kernel_phys kernel_main_image)

add_subdirectory("lib")
