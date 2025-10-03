# Test Configuration for TDD Implementation
# Following IEEE 12207-2017 Implementation Process + XP TDD practices

# Test executable for Platform Abstraction Layer (DES-C-004)
add_executable(test_platform
    platform/test_platform_factory.cpp
    platform/test_filesystem.cpp
    platform/test_threading.cpp
    platform/test_memory_manager.cpp
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

# Test executable for Data Access Layer (DES-C-003)
add_executable(test_data
    data/test_data_access_engine.cpp
    data/test_xml_processor.cpp
    data/test_zip_processor.cpp
)

target_link_libraries(test_data
    PRIVATE 
        dawproject_data
        Catch2::Catch2WithMain
)

target_include_directories(test_data PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_data)

# Test executable for Business Logic Layer (DES-C-002) 
add_executable(test_business
    business/test_project_engine.cpp
    business/test_domain_model.cpp
    business/test_validation_engine.cpp
)

target_link_libraries(test_business
    PRIVATE 
        dawproject_business
        Catch2::Catch2WithMain
)

target_include_directories(test_business PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

catch_discover_tests(test_business)

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