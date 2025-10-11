# Test Configuration for TDD Implementation
# Following IEEE 12207-2017 Implementation Process + XP TDD practices

# Test executable for Platform Abstraction Layer (DES-C-004)
add_executable(test_platform
    unit/test_platform_filesystem.cpp
    unit/test_platform_threading.cpp
    unit/test_platform_memory.cpp
)

target_link_libraries(test_platform
    PRIVATE 
        dawproject_platform
        Catch2::Catch2WithMain
)

target_include_directories(test_platform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Register tests with CTest
catch_discover_tests(test_platform)

# Comprehensive Platform Layer Tests for Coverage Improvement
add_executable(test_platform_comprehensive
    unit/platform/test_platform_comprehensive.cpp
    unit/platform/test_platform_threading_memory.cpp
)

target_link_libraries(test_platform_comprehensive
    PRIVATE 
        dawproject_platform
        Catch2::Catch2WithMain
)

target_include_directories(test_platform_comprehensive PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_platform_comprehensive)

# Test executable for Data Access Layer (DES-C-003) - Using existing tests
add_executable(test_data_existing
    unit/test_data_access_red.cpp
)

target_link_libraries(test_data_existing
    PRIVATE 
        dawproject_data
        Catch2::Catch2WithMain
)

target_include_directories(test_data_existing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_data_existing)

# Comprehensive Data Access Tests for Coverage Improvement
add_executable(test_data_comprehensive
    unit/data/test_data_access_comprehensive.cpp
)

target_link_libraries(test_data_comprehensive
    PRIVATE 
        dawproject_data
        Catch2::Catch2WithMain
)

target_include_directories(test_data_comprehensive PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_data_comprehensive)

# Test executable for Business Logic Layer (DES-C-002) - Mock implementation
add_executable(test_business_mock
    unit/test_smoke_coverage.cpp
)

target_link_libraries(test_business
    PRIVATE 
        dawproject_business
        Catch2::Catch2WithMain
)

# Comprehensive Business Logic and Integration Tests
add_executable(test_business_integration
    integration/test_business_integration.cpp
)

target_link_libraries(test_business_integration
    PRIVATE 
        dawproject_business
        dawproject_data
        dawproject_external_validation
        Catch2::Catch2WithMain
)

target_include_directories(test_business PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_business)

target_include_directories(test_business_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_business_integration)

# Test executable for API Layer (DES-C-001)
add_executable(test_api
    api/test_cpp_api.cpp
    api/test_c_api.cpp
    api/test_error_handling.cpp
)

target_link_libraries(test_api
    PRIVATE 
        dawproject_api
        Catch2::Catch2WithMain
)

target_include_directories(test_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_api)

# Integration tests
add_executable(test_integration
    integration/test_end_to_end.cpp
    integration/test_interface_contracts.cpp
    integration/test_performance.cpp
)

target_link_libraries(test_integration
    PRIVATE 
        dawproject_api
        Catch2::Catch2WithMain
)

target_include_directories(test_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_integration)

# Enable CTest
enable_testing()