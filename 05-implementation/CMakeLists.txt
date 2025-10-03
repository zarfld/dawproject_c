cmake_minimum_required(VERSION 3.20)

# Project configuration following IEEE standards and XP practices
project(dawproject_c
    VERSION 1.0.0
    DESCRIPTION "DAW Project C++ Library - Standards-Compliant Implementation"
    LANGUAGES CXX
)

# C++ Standards (per ADR-002)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# TDD and Quality Options
option(ENABLE_TESTING "Enable unit and integration testing" ON)
option(ENABLE_COVERAGE "Enable code coverage analysis" ON)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" ON)

# Global compiler flags for quality and debugging
if(MSVC)
    # MSVC specific flags
    add_compile_options(/W4 /WX /permissive- /Zc:__cplusplus)
    if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    endif()
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Werror -pedantic)
    if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
    if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Enabling coverage instrumentation")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# External dependencies management
include(FetchContent)

# Catch2 for testing (following TDD strategy)
if(ENABLE_TESTING)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
    
    # Add Catch2's CMake modules to path
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
endif()

# pugixml for XML processing (per DES-C-003)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

# Create interface library for common compile features
add_library(dawproject_common INTERFACE)
target_compile_features(dawproject_common INTERFACE cxx_std_17)
target_compile_definitions(dawproject_common INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    UNICODE
    _UNICODE
)

# Platform Abstraction Layer (DES-C-004) - Minimal for TDD
add_library(dawproject_platform STATIC)
target_sources(dawproject_platform
    PRIVATE
        src/platform/filesystem_impl.cpp
        src/platform/memory_impl.cpp
        src/platform/platform_factory.cpp
    PUBLIC
        include/dawproject/platform/interfaces.h
        include/dawproject/platform/factory.h
)
target_link_libraries(dawproject_platform 
    PUBLIC dawproject_common
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
)

# Data Access Layer (DES-C-003) 
add_library(dawproject_data STATIC)
target_sources(dawproject_data
    PRIVATE
        src/data/data_access_factory.cpp
        src/data/data_access_engine_impl.cpp
        src/data/xml_processor_impl.cpp
        src/data/zip_processor_impl.cpp
        src/data/data_access_engine_extracted.cpp
    PUBLIC
        include/dawproject/data/data_access_engine.h
        include/dawproject/data/data_access_factory.h
)
target_link_libraries(dawproject_data
    PUBLIC dawproject_platform dawproject_common
    PRIVATE pugixml
)

# Enable testing
if(ENABLE_TESTING)
    enable_testing()
    
    # RED phase test for data access layer
    add_executable(test_data_red
        tests/unit/test_data_access_red.cpp
    )
    target_link_libraries(test_data_red
        PRIVATE dawproject_data Catch2::Catch2WithMain
    )
    catch_discover_tests(test_data_red)
    
    # RED phase validation runner
    add_executable(red_phase_runner
        red_phase_runner.cpp
    )
    target_link_libraries(red_phase_runner
        PRIVATE dawproject_data
    )
    
    # Custom test target for TDD workflow
    add_custom_target(tdd
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
        DEPENDS test_data_red
        COMMENT "Running TDD test suite"
    )
endif()

# Performance testing executable (for refactoring verification)
if(ENABLE_TESTING)
    add_executable(performance_test
        src/performance_test.cpp
    )
    
    target_include_directories(performance_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    target_link_libraries(performance_test PRIVATE
        dawproject_data
    )
    
    # Performance test command
    add_custom_target(run_performance_test
        COMMAND performance_test
        DEPENDS performance_test
        COMMENT "Running performance verification tests"
    )
endif()

# Print configuration summary
message(STATUS "DAW Project C++ Library Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Testing enabled: ${ENABLE_TESTING}")
message(STATUS "  Code coverage: ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers: ${ENABLE_SANITIZERS}")