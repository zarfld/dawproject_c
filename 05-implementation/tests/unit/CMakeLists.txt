# CMake configuration for running TDD tests
# This allows us to run: cmake --build . --target test

# Simplified CMakeLists.txt for initial TDD cycle
cmake_minimum_required(VERSION 3.20)
project(dawproject_tdd_phase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../include)

# Fetch Catch2 for testing
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Create minimal platform library for testing
add_library(dawproject_platform_minimal
    ../src/platform/platform_factory.cpp
    ../src/platform/filesystem_impl.cpp
    ../src/platform/threading_impl.cpp
    ../src/platform/memory_impl.cpp
)
target_include_directories(dawproject_platform_minimal PUBLIC ../../include)
target_compile_definitions(dawproject_platform_minimal PRIVATE _CRT_SECURE_NO_WARNINGS)

# Create test executable
add_executable(test_platform_red
    test_platform_filesystem.cpp
    test_platform_threading.cpp
    test_platform_memory.cpp
)

target_link_libraries(test_platform_red 
    PRIVATE dawproject_platform_minimal Catch2::Catch2WithMain
)

# Enable testing
enable_testing()
add_test(NAME PlatformTests COMMAND test_platform_red)