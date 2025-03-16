file(GLOB LIBRARIES
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}" 
             "${CMAKE_CURRENT_LIST_DIR}/*/CMakeLists.txt"
)
foreach(library IN LISTS LIBRARIES)
    get_filename_component(library_dir "${library}" DIRECTORY)
    add_subdirectory(${library_dir})
endforeach()