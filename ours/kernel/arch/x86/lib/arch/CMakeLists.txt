target_include_directories(libarch_headers
INTERFACE
    "include"
)

target_sources(libarch 
INTERFACE 
    "processor.cpp"
    "page_table.cpp"
    "pic.cpp"
)

target_compile_options(libarch
INTERFACE
    "-mrdrnd"
)