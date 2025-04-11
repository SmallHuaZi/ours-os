add_library(eftl INTERFACE)
add_library(kernel::lib::eftl ALIAS eftl)
target_include_directories(eftl 
INTERFACE
    "include"
)
target_sources(eftl 
INTERFACE
    "placeholder.cpp"
)