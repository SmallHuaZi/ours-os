message("Enter PreDefines.cmake")

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") 
    set(OURS_ARCH "x86")
else()
    set(OURS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

include(${OURS_ARCH}PreDefines)

add_definitions("-DOURS_DEV=1")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DOURS_DEBUG=1")
else (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions("-DOURS_RELEASE=1")
endif ()

add_definitions("-DOURS_CONFIG_NUMA=1")

message("Leave PreDefines.cmake")