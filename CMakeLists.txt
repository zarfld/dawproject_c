cmake_minimum_required(VERSION 3.15)

# Project configuration
project(dawproject_cpp
    VERSION 1.0.0
    DESCRIPTION "C++ library for reading, writing, and manipulating Bitwig DAW Project files"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(DAWPROJECT_BUILD_TESTS "Build tests" ON)
option(DAWPROJECT_BUILD_EXAMPLES "Build examples" ON)
option(DAWPROJECT_BUILD_DOCS "Build documentation" OFF)
option(DAWPROJECT_ENABLE_COVERAGE "Enable code coverage" OFF)

# Configuration for different build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
if(MSVC)
    # MSVC-specific flags
    add_compile_options(/W4 /WX)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    endif()
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Werror -pedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
    
    # Address sanitizer for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# Find dependencies
find_package(PkgConfig QUIET)

# Threading support (required for thread-safe operations)
find_package(Threads REQUIRED)

# Set up directories
set(DAWPROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/05-implementation/include)
set(DAWPROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/05-implementation/src)
set(DAWPROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/05-implementation/examples)
set(DAWPROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/05-implementation/tests)

# Include directories
include_directories(${DAWPROJECT_INCLUDE_DIR})

# Placeholder for library sources (to be added in Phase 05)
set(DAWPROJECT_SOURCES
    # Core library sources will be added during implementation phase
    # ${DAWPROJECT_SOURCE_DIR}/DawProject.cpp
    # ${DAWPROJECT_SOURCE_DIR}/Project.cpp
    # ${DAWPROJECT_SOURCE_DIR}/Track.cpp
    # ${DAWPROJECT_SOURCE_DIR}/Clip.cpp
)

# Placeholder for public headers
set(DAWPROJECT_HEADERS
    # Public headers will be added during implementation phase
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/DawProject.h
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/Project.h
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/Track.h
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/Clip.h
)

# Create library target (placeholder for Phase 05)
# add_library(dawproject ${DAWPROJECT_SOURCES} ${DAWPROJECT_HEADERS})
# target_include_directories(dawproject PUBLIC ${DAWPROJECT_INCLUDE_DIR})
# target_link_libraries(dawproject Threads::Threads)

# Set up exports
# set_target_properties(dawproject PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION 1
#     PUBLIC_HEADER "${DAWPROJECT_HEADERS}"
# )

# Examples
if(DAWPROJECT_BUILD_EXAMPLES)
    # add_subdirectory(${DAWPROJECT_EXAMPLE_DIR})
    message(STATUS "Examples will be built in Phase 05")
endif()

# Tests
if(DAWPROJECT_BUILD_TESTS)
    enable_testing()
    # add_subdirectory(${DAWPROJECT_TEST_DIR})
    message(STATUS "Tests will be configured in Phase 05")
endif()

# Documentation
if(DAWPROJECT_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        # add_subdirectory(docs)
        message(STATUS "Documentation will be configured in Phase 05")
    else()
        message(WARNING "Doxygen not found, documentation cannot be built")
    endif()
endif()

# Installation (placeholder)
# install(TARGETS dawproject
#     EXPORT dawproject-targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     PUBLIC_HEADER DESTINATION include/dawproject
# )

# Export configuration
# install(EXPORT dawproject-targets
#     FILE dawproject-config.cmake
#     DESTINATION lib/cmake/dawproject
# )

# Package configuration
set(CPACK_PACKAGE_NAME "dawproject-cpp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "DAW Project C++ Implementation")
set(CPACK_PACKAGE_CONTACT "zarfld@github.com")

include(CPack)

# Status messages
message(STATUS "")
message(STATUS "DAW Project C++ Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Tests: ${DAWPROJECT_BUILD_TESTS}")
message(STATUS "  Build Examples: ${DAWPROJECT_BUILD_EXAMPLES}")
message(STATUS "  Build Documentation: ${DAWPROJECT_BUILD_DOCS}")
message(STATUS "  Enable Coverage: ${DAWPROJECT_ENABLE_COVERAGE}")
message(STATUS "")
message(STATUS "Current Phase: Phase 01-02 (Requirements)")
message(STATUS "Implementation will begin in Phase 05")
message(STATUS "")