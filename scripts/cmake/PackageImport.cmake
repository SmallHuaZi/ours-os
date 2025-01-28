# https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)

FetchContent_Declare(
    ustl
    GIT_REPOSITORY https://github.com/smallhuazi/ustl.git
    GIT_TAG ustl-0.0.1
    FIND_PACKAGE_ARGS NAMES ustl 
)
FetchContent_MakeAvailable(ustl)

