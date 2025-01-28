add_library(ours_system_public_headers INTERFACE)
add_library(ours::system::public::headers ALIAS ours_system_public_headers)
target_include_directories(ours_system_public_headers 
INTERFACE
    "include"
)