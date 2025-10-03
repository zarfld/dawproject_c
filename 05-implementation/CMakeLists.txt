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
option(ENABLE_STATIC_ANALYSIS "Enable clang-tidy static analysis" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

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
endif()

# Code coverage flags
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/minizip
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
    GIT_TAG v1.13
)
FetchContent_MakeAvailable(pugixml)

# minizip for ZIP processing (per DES-C-003)
find_package(ZLIB REQUIRED)

# Create interface library for common compile features
add_library(dawproject_common INTERFACE)
target_compile_features(dawproject_common INTERFACE cxx_std_17)
target_compile_definitions(dawproject_common INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    UNICODE
    _UNICODE
)

# Platform Abstraction Layer (DES-C-004)
add_library(dawproject_platform)
target_sources(dawproject_platform
    PRIVATE
        src/platform/filesystem.cpp
        src/platform/threading.cpp 
        src/platform/memory_manager.cpp
        src/platform/error_adapter.cpp
        src/platform/platform_factory.cpp
    PUBLIC
        include/dawproject/platform/interfaces.h
        include/dawproject/platform/factory.h
        include/dawproject/platform/types.h
)
target_link_libraries(dawproject_platform 
    PUBLIC dawproject_common
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
)

# Data Access Layer (DES-C-003) 
add_library(dawproject_data)
target_sources(dawproject_data
    PRIVATE
        src/data/xml_processor.cpp
        src/data/zip_processor.cpp
        src/data/data_access_engine.cpp
        src/data/project_reader.cpp
        src/data/project_writer.cpp
    PUBLIC
        include/dawproject/data/interfaces.h
        include/dawproject/data/types.h
)
target_link_libraries(dawproject_data
    PUBLIC dawproject_platform dawproject_common
    PRIVATE pugixml::pugixml ZLIB::ZLIB
)

# Business Logic Layer (DES-C-002)
add_library(dawproject_business)
target_sources(dawproject_business 
    PRIVATE
        src/business/project_engine.cpp
        src/business/domain_model.cpp
        src/business/validation_engine.cpp
        src/business/concurrency_manager.cpp
        src/business/transformation_engine.cpp
    PUBLIC
        include/dawproject/business/interfaces.h
        include/dawproject/business/types.h
)
target_link_libraries(dawproject_business
    PUBLIC dawproject_data dawproject_common
)

# API Layer (DES-C-001)
add_library(dawproject_api)
target_sources(dawproject_api
    PRIVATE
        src/api/cpp_api.cpp
        src/api/c_api.cpp
        src/api/handle_manager.cpp
        src/api/error_translator.cpp
        src/api/parameter_validator.cpp
    PUBLIC
        include/dawproject/dawproject.h
        include/dawproject/dawproject_c.h
        include/dawproject/api/types.h
)
target_link_libraries(dawproject_api
    PUBLIC dawproject_business dawproject_common
)

# Main library combining all layers
add_library(dawproject)
target_sources(dawproject
    PRIVATE
        src/dawproject.cpp
    PUBLIC
        include/dawproject/dawproject.h
        include/dawproject/dawproject_c.h
)
target_link_libraries(dawproject
    PUBLIC dawproject_api dawproject_business dawproject_data dawproject_platform
)

# Enable testing
if(ENABLE_TESTING)
    enable_testing()
    
    # Unit tests for each layer (TDD approach)
    add_executable(test_platform
        tests/unit/test_platform_filesystem.cpp
        tests/unit/test_platform_threading.cpp
        tests/unit/test_platform_memory.cpp
    )
    target_link_libraries(test_platform 
        PRIVATE dawproject_platform Catch2::Catch2WithMain
    )
    catch_discover_tests(test_platform)
    
    add_executable(test_data
        tests/unit/test_data_xml.cpp
        tests/unit/test_data_zip.cpp
        tests/unit/test_data_engine.cpp
    )
    target_link_libraries(test_data
        PRIVATE dawproject_data Catch2::Catch2WithMain
    )
    catch_discover_tests(test_data)
    
    add_executable(test_business
        tests/unit/test_business_engine.cpp
        tests/unit/test_business_domain.cpp
        tests/unit/test_business_validation.cpp
    )
    target_link_libraries(test_business
        PRIVATE dawproject_business Catch2::Catch2WithMain
    )
    catch_discover_tests(test_business)
    
    add_executable(test_api
        tests/unit/test_api_cpp.cpp
        tests/unit/test_api_c.cpp
        tests/unit/test_api_handles.cpp
    )
    target_link_libraries(test_api
        PRIVATE dawproject_api Catch2::Catch2WithMain
    )
    catch_discover_tests(test_api)
    
    # Integration tests
    add_executable(test_integration
        tests/integration/test_end_to_end.cpp
        tests/integration/test_performance.cpp
        tests/integration/test_thread_safety.cpp
    )
    target_link_libraries(test_integration
        PRIVATE dawproject Catch2::Catch2WithMain
    )
    catch_discover_tests(test_integration)
    
    # Custom test target for TDD workflow
    add_custom_target(tdd
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
        DEPENDS test_platform test_data test_business test_api test_integration
        COMMENT "Running TDD test suite"
    )
endif()

# Static analysis with clang-tidy
if(ENABLE_STATIC_ANALYSIS)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(dawproject_platform dawproject_data dawproject_business dawproject_api
            PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*"
        )
    endif()
endif()

# Installation configuration
include(GNUInstallDirs)
install(TARGETS dawproject dawproject_api dawproject_business dawproject_data dawproject_platform
    EXPORT dawproject-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/dawproject
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Package configuration
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/dawproject-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/dawproject-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawproject
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/dawproject-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dawproject-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/dawproject-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawproject
)

install(EXPORT dawproject-targets
    FILE dawproject-targets.cmake
    NAMESPACE dawproject::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawproject
)

# Print configuration summary
message(STATUS "DAW Project C++ Library Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Testing enabled: ${ENABLE_TESTING}")
message(STATUS "  Code coverage: ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "  Static analysis: ${ENABLE_STATIC_ANALYSIS}")