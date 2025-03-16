add_library(system_public_headers INTERFACE)
add_library(system::public::headers ALIAS system_public_headers)
target_include_directories(system_public_headers 
INTERFACE
    "include"
)