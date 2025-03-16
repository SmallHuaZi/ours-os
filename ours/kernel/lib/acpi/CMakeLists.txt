cmake_minimum_required(VERSION 3.20)
project(acpi VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(acpi_headers INTERFACE)
add_library(kernel::lib::acpi::headers ALIAS acpi_headers)
target_include_directories(acpi_headers 
INTERFACE 
    "include"
)
target_link_libraries(acpi_headers
INTERFACE
    kernel::main::headers
)

add_library(acpi INTERFACE)
add_library(kernel::lib::acpi ALIAS acpi)
target_sources(acpi 
INTERFACE
    "apic.cpp"
    "numa.cpp"
    "debug.cpp"
    "parser.cpp"
)
target_link_libraries(acpi
INTERFACE
    acpi_headers
)