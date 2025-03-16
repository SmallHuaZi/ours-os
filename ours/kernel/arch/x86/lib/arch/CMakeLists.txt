target_include_directories(libarch_headers
INTERFACE
    "include"
)

target_sources(libarch 
PRIVATE 
    "page_table.cpp"
)
