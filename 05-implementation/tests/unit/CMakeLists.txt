# CMake configuration for running TDD tests
# This allows us to run: cmake --build . --target test

# Simplified CMakeLists.txt for initial TDD cycle
cmake_minimum_required(VERSION 3.20)
project(dawproject_tdd_phase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External validation options
option(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION "Enable external XSD validation with libxml2" ON)

# Find libxml2 for external validation (OPTIONAL for REFACTOR demonstration)
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBXML2 libxml-2.0)
        if(LIBXML2_FOUND)
            message(STATUS "Found libxml2 ${LIBXML2_VERSION} (enabled for external XSD validation)")
        else()
            message(WARNING "libxml2 not found via pkg-config - using fallback validation")
        endif()
    else()
        find_package(LibXml2 QUIET)
        if(LIBXML2_FOUND)
            message(STATUS "Found libxml2 (enabled for external XSD validation)")
        else()
            message(WARNING "libxml2 not found - using fallback validation")
        endif()
    endif()
else()
    message(WARNING "External XSD validation disabled - may violate external authority compliance")
endif()

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
    ../../src/platform/platform_factory.cpp
    ../../src/platform/filesystem_impl.cpp
    ../../src/platform/threading_impl.cpp
    ../../src/platform/memory_impl.cpp
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

# Create external validation components only if enabled
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION)
    # Create external validation service library for testing (Domain-Driven Architecture)
    add_library(dawproject_external_validation_ddd
        ../../src/application/external-validation/external_validation_service.cpp
        ../../src/infrastructure/xml-validation/libxml2_validator.cpp
        ../../src/core/error_info.cpp
    )
    target_include_directories(dawproject_external_validation_ddd PUBLIC ../../include)
    target_compile_definitions(dawproject_external_validation_ddd PRIVATE _CRT_SECURE_NO_WARNINGS)

    # Link libxml2 to the external validation library if available
    if(LIBXML2_FOUND)
        if(PkgConfig_FOUND)
            target_link_libraries(dawproject_external_validation_ddd ${LIBXML2_LIBRARIES})
            target_include_directories(dawproject_external_validation_ddd PRIVATE ${LIBXML2_INCLUDE_DIRS})
            target_compile_definitions(dawproject_external_validation_ddd PRIVATE DAWPROJECT_HAS_LIBXML2)
        else()
            target_link_libraries(dawproject_external_validation_ddd LibXml2::LibXml2)
            target_compile_definitions(dawproject_external_validation_ddd PRIVATE DAWPROJECT_HAS_LIBXML2)
        endif()
    endif()

    # Create external validation test executable (Domain-Driven Architecture)
    add_executable(test_external_validation_ddd
        application/external-validation/test_external_validation_service.cpp
    )

    target_link_libraries(test_external_validation_ddd
        PRIVATE dawproject_external_validation_ddd Catch2::Catch2WithMain
    )
endif()

# Enable testing
enable_testing()
add_test(NAME PlatformTests COMMAND test_platform_red)

# Add external validation tests only if enabled
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION)
    add_test(NAME ExternalValidationTests COMMAND test_external_validation_ddd)
endif()