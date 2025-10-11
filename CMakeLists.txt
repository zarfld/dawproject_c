cmake_minimum_required(VERSION 3.15)

# Project configuration
project(dawproject_cpp
    VERSION 1.0.0
    DESCRIPTION "C++ library for reading, writing, and manipulating Bitwig DAW Project files"
    LANGUAGES CXX
)

# C++ standard (ISO/IEC/IEEE 12207:2017 Implementation Process)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# XP Practices: Simple Design, Clean Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For IDE integration and code analysis

# Build options (TDD/XP Practices)
option(DAWPROJECT_BUILD_TESTS "Build tests (TDD practice)" ON)
option(DAWPROJECT_BUILD_EXAMPLES "Build examples" ON)
option(DAWPROJECT_BUILD_DOCS "Build documentation" OFF)
option(DAWPROJECT_ENABLE_COVERAGE "Enable code coverage (>90% target)" OFF)
option(DAWPROJECT_ENABLE_EXTERNAL_AUTHORITY "Enable external authority compliance (MANDATORY)" ON)
option(DAWPROJECT_ENABLE_TDD_MOCKS "Enable TDD mocking support" ON)

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

# External Authority Compliance Dependencies
option(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION "Enable external XSD validation with libxml2" ON)
option(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT "Enable external schema download with libcurl" ON)

# External XSD validation support (libxml2) - CRITICAL for external authority compliance
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
        if(LIBXML2_FOUND)
            message(STATUS "Found libxml2 ${LIBXML2_VERSION} (required for external XSD validation)")
        endif()
    else()
        find_package(LibXml2 REQUIRED)
        if(LIBXML2_FOUND)
            message(STATUS "Found libxml2 (required for external XSD validation)")
        endif()
    endif()
else()
    message(WARNING "External XSD validation disabled - may violate external authority compliance")
endif()

# HTTP client for external schema management (libcurl) - CRITICAL for external authority compliance
if(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT)
    find_package(CURL REQUIRED)
    if(CURL_FOUND)
        message(STATUS "Found libcurl ${CURL_VERSION_STRING} (required for external schema management)")
    endif()
else()
    message(WARNING "External schema management disabled - may violate external authority compliance")
endif()

# pugixml for fast XML parsing (header-only preferred)
find_path(PUGIXML_INCLUDE_DIR pugixml.hpp
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pugixml/src
    PATHS /usr/include /usr/local/include
    DOC "Path to pugixml header"
)
if(PUGIXML_INCLUDE_DIR)
    message(STATUS "Found pugixml at ${PUGIXML_INCLUDE_DIR}")
else()
    message(STATUS "pugixml not found - will be downloaded automatically")
endif()

# ZIP library for DAWProject archive handling
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBZIP libzip)
    if(LIBZIP_FOUND)
        message(STATUS "Found libzip ${LIBZIP_VERSION}")
    else()
        message(STATUS "libzip not found via pkg-config - will use fallback")
    endif()
endif()

# Set up directories
set(DAWPROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAWPROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DAWPROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Include directories
include_directories(${DAWPROJECT_INCLUDE_DIR})

# Phase 05 Implementation - Core library sources (TDD approach)
set(DAWPROJECT_SOURCES
    # Core Result<T> Pattern Implementation
    ${DAWPROJECT_SOURCE_DIR}/core/error_info.cpp
    
    # Additional sources will be added as we implement them following TDD
    # ${DAWPROJECT_SOURCE_DIR}/external/external_schema_manager.cpp
    # ${DAWPROJECT_SOURCE_DIR}/data/validation_engine.cpp
    # ${DAWPROJECT_SOURCE_DIR}/data/data_access_layer.cpp
    # ${DAWPROJECT_SOURCE_DIR}/interop/interoperability_validator.cpp
)

# Phase 05 Implementation - Public headers
set(DAWPROJECT_HEADERS
    # Core interfaces and patterns
    ${DAWPROJECT_INCLUDE_DIR}/dawproject/core/result.hpp
    
    # Additional headers will be added as we implement them
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/external/external_authority.hpp
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/data/validation_interfaces.hpp
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/data/data_access_interfaces.hpp
    # ${DAWPROJECT_INCLUDE_DIR}/dawproject/interop/interoperability_interfaces.hpp
)

# Create library target - Phase 05 Implementation
add_library(dawproject ${DAWPROJECT_SOURCES} ${DAWPROJECT_HEADERS})
target_include_directories(dawproject PUBLIC ${DAWPROJECT_INCLUDE_DIR})
target_compile_features(dawproject PUBLIC cxx_std_20)  # C++20 required

# Core dependencies
target_link_libraries(dawproject Threads::Threads)

# External authority compliance dependencies
if(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT AND LIBXML2_FOUND)
    if(PkgConfig_FOUND AND LIBXML2_FOUND)
        target_link_libraries(dawproject ${LIBXML2_LIBRARIES})
        target_include_directories(dawproject PRIVATE ${LIBXML2_INCLUDE_DIRS})
#         target_compile_definitions(dawproject PRIVATE DAWPROJECT_HAS_LIBXML2)
#     else()
#         target_link_libraries(dawproject LibXml2::LibXml2)
#         target_compile_definitions(dawproject PRIVATE DAWPROJECT_HAS_LIBXML2)
#     endif()
# endif()
# 
# if(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT AND CURL_FOUND)
#     target_link_libraries(dawproject CURL::libcurl)
#     target_compile_definitions(dawproject PRIVATE DAWPROJECT_HAS_CURL)
# endif()
# 
# # pugixml integration
# if(PUGIXML_INCLUDE_DIR)
#     target_include_directories(dawproject PRIVATE ${PUGIXML_INCLUDE_DIR})
#     target_compile_definitions(dawproject PRIVATE DAWPROJECT_HAS_PUGIXML)
# endif()
# 
# # ZIP library integration
# if(LIBZIP_FOUND)
#     target_link_libraries(dawproject ${LIBZIP_LIBRARIES})
#     target_include_directories(dawproject PRIVATE ${LIBZIP_INCLUDE_DIRS})
#     target_compile_definitions(dawproject PRIVATE DAWPROJECT_HAS_LIBZIP)
# endif()

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
message(STATUS "External Authority Compliance:")
message(STATUS "  External XSD Validation: ${DAWPROJECT_ENABLE_EXTERNAL_VALIDATION}")
message(STATUS "  External Schema Management: ${DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT}")
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION AND LIBXML2_FOUND)
    message(STATUS "  ✓ libxml2 found - external XSD validation available")
else()
    message(STATUS "  ⚠ libxml2 missing - external XSD validation unavailable")
endif()
if(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT AND CURL_FOUND)
    message(STATUS "  ✓ libcurl found - external schema management available")
else()
    message(STATUS "  ⚠ libcurl missing - external schema management unavailable")
endif()
message(STATUS "")
message(STATUS "Current Phase: Phase 03 (Architecture - External Authority Updated)")
message(STATUS "Implementation will begin in Phase 05")
message(STATUS "")