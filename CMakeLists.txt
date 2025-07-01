cmake_minimum_required(VERSION 3.29.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DEBUG "Enable debug mode" ON)
option(RELEASE "Enable release mode" OFF)

# Only set project if this is the main project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(STANDALONE_BUILD TRUE)
    project(pitch-shift)

    # Only set build type if this is the main project
    if(STANDALONE_BUILD AND DEBUG)
        message(STATUS "Debug build enabled")
        set(CMAKE_BUILD_TYPE RelWithDebInfo)
    else()
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

# Only enable testing and FetchContent if this is the main project
if(STANDALONE_BUILD AND DEBUG)
    # Pull in Google Test
    message(STATUS "Unit tests enabled")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2)
    FetchContent_MakeAvailable(GoogleTest)
    enable_testing()
    include(GoogleTest)
    include("cmake/create_ut.cmake")
endif()

add_subdirectory(src/common)