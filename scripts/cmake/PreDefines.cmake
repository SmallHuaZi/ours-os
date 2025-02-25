message("Enter PreDefines.cmake")

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") 
    set(OURS_ARCH "x86")
    add_definitions("-DX86_PAGING_LEVEL=4")
else()
    set(OURS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

add_definitions("-DOURS_DEV")
add_definitions("-DBOOST_CONTAINER_USER_DEFINED_THROW_CALLBACKS=1")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DOURS_DEBUG=1")
else (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions("-DOURS_RELEASE=1")
endif ()

add_definitions("-DOURS_CONFIG_NUMA=1")

message("Leave PreDefines.cmake")