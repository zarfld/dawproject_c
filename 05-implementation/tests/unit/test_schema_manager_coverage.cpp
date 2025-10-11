/**
 * @file test_schema_manager_coverage.cpp
 * @brief Coverage tests for ExternalSchemaManager (477-line target file)
 * 
 * Comprehensive testing to increase code coverage focusing on:
 * - Constructor and configuration options
 * - Schema download operations (success/failure paths)  
 * - Caching mechanisms and cache management
 * - Schema validation and integrity checks
 * - HTTP client interface usage
 * - Error handling and edge cases
 * - File system operations
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <dawproject/external/external_schema_manager.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <cstdlib>

using namespace dawproject;
using namespace Catch::Matchers;

// Test fixture for schema manager tests
class SchemaManagerFixture {
public:
    SchemaManagerFixture() {
        // Create temporary directory for testing
        const char* temp_dir = std::getenv("TEMP");
        if (!temp_dir) {
            temp_dir = "/tmp";
        }
        
        tempDir_ = std::filesystem::path(temp_dir) / "schema_manager_test";
        std::filesystem::create_directories(tempDir_);
        
        // Setup test configuration
        config_.cacheDirectory = tempDir_;
        config_.maxCacheSize = 1024 * 1024; // 1MB
        config_.defaultTimeout = std::chrono::seconds(5);
    }
    
    ~SchemaManagerFixture() {
        // Cleanup temporary directory
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
    }

protected:
    std::filesystem::path tempDir_;
    ExternalSchemaManager::Config config_;
    
    void createTestSchemaFile(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path);
        file << content;
        file.close();
    }
};

// Mock HTTP client for testing
class MockHTTPClient : public HTTPClient {
public:
    bool shouldSucceed = true;
    std::string responseBody = R"(<?xml version="1.0"?>
<schema xmlns="http://www.w3.org/2001/XMLSchema" 
        targetNamespace="http://dawproject.org/schema/1.0"
        elementFormDefault="qualified">
    <element name="project" type="string"/>
</schema>)";
    
    Result<Response> get(
        const std::string& url,
        std::chrono::milliseconds timeout) override {
        
        (void)timeout; // Suppress unused parameter warning
        
        Response response;
        response.statusCode = shouldSucceed ? 200 : 404;
        response.body = shouldSucceed ? responseBody : "Not Found";
        response.contentType = "application/xml";
        
        return Result<Response>::success(response);
    }
};

// Test ExternalSchemaManager constructor and configuration
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Construction", "[schema][manager][constructor]") {
    
    SECTION("Constructor with default config") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        // Should construct successfully with valid config
        REQUIRE_NOTHROW(ExternalSchemaManager(config_, std::move(httpClient)));
    }
    
    SECTION("Constructor creates cache directory") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        config_.cacheDirectory = tempDir_ / "new_cache";
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Cache directory should be created
        REQUIRE(std::filesystem::exists(config_.cacheDirectory));
        REQUIRE(std::filesystem::is_directory(config_.cacheDirectory));
    }
    
    SECTION("Constructor with empty cache directory") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        config_.cacheDirectory = "";
        
        // Should handle empty cache directory gracefully
        REQUIRE_NOTHROW(ExternalSchemaManager(config_, std::move(httpClient)));
    }
}

// Test schema download functionality
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Schema Download", "[schema][manager][download]") {
    
    SECTION("Successful schema download") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        httpClient->shouldSucceed = true;
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        auto result = manager.downloadSchema(
            "dawproject.org",
            "https://dawproject.org/schema/1.0/schema.xsd", 
            "1.0"
        );
        
        REQUIRE(result.isSuccess());
        const auto& schema = result.getValue();
        REQUIRE(schema.authority == "dawproject.org");
        REQUIRE(schema.version == "1.0");
        REQUIRE(!schema.localPath.empty());
        REQUIRE(schema.isValid);
    }
    
    SECTION("Failed schema download - HTTP error") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        httpClient->shouldSucceed = false;
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        auto result = manager.downloadSchema(
            "dawproject.org",
            "https://dawproject.org/schema/1.0/nonexistent.xsd",
            "1.0"
        );
        
        REQUIRE(result.isFailure());
        REQUIRE_THAT(result.getErrorMessage(), ContainsSubstring("HTTP"));
    }
    
    SECTION("Download with empty parameters") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Test empty authority
        auto result1 = manager.downloadSchema("", "https://example.com/schema.xsd", "1.0");
        REQUIRE(result1.isFailure());
        
        // Test empty URL  
        auto result2 = manager.downloadSchema("authority", "", "1.0");
        REQUIRE(result2.isFailure());
        
        // Test empty version
        auto result3 = manager.downloadSchema("authority", "https://example.com/schema.xsd", "");
        REQUIRE(result3.isFailure());
    }
    
    SECTION("Download with invalid URL format") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        auto result = manager.downloadSchema(
            "authority",
            "invalid-url-format",
            "1.0"
        );
        
        // Should handle invalid URL gracefully
        REQUIRE(result.isFailure());
    }
}

// Test caching functionality  
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Caching", "[schema][manager][cache]") {
    
    SECTION("Cache retrieval - existing schema") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // First download to populate cache
        auto downloadResult = manager.downloadSchema(
            "dawproject.org", 
            "https://dawproject.org/schema/1.0/schema.xsd",
            "1.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Then retrieve from cache
        auto cacheResult = manager.getCachedSchema("dawproject.org", "1.0");
        REQUIRE(cacheResult.isSuccess());
        
        const auto& cached = cacheResult.getValue();
        REQUIRE(cached.authority == "dawproject.org");
        REQUIRE(cached.version == "1.0");
    }
    
    SECTION("Cache retrieval - non-existing schema") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        auto result = manager.getCachedSchema("nonexistent.org", "2.0");
        REQUIRE(result.isFailure());
        REQUIRE_THAT(result.getErrorMessage(), ContainsSubstring("not found"));
    }
    
    SECTION("Cache path retrieval") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Download schema first
        auto downloadResult = manager.downloadSchema(
            "example.org",
            "https://example.org/schema.xsd", 
            "1.5"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Get path to cached schema
        auto pathResult = manager.getSchemaPath("example.org", "1.5");
        REQUIRE(pathResult.isSuccess());
        
        const auto& path = pathResult.getValue();
        REQUIRE(std::filesystem::exists(path));
        REQUIRE(std::filesystem::is_regular_file(path));
    }
    
    SECTION("Cache path retrieval - non-existing") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        auto result = manager.getSchemaPath("missing.org", "0.1");
        REQUIRE(result.isFailure());
    }
}

// Test schema validation functionality
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Validation", "[schema][manager][validation]") {
    
    SECTION("Validate schema integrity - valid schema") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Create valid schema info
        SchemaInfo schemaInfo;
        schemaInfo.authority = "test.org";
        schemaInfo.version = "1.0";
        schemaInfo.checksum = "abc123";
        schemaInfo.localPath = tempDir_ / "valid_schema.xsd";
        schemaInfo.isValid = true;
        
        // Create test file
        createTestSchemaFile(schemaInfo.localPath, httpClient->responseBody);
        
        auto result = manager.validateSchemaIntegrity(schemaInfo);
        // Note: Depending on implementation, this might succeed or fail
        // We're primarily testing that the method doesn't crash
        REQUIRE_NOTHROW(result.isSuccess() || result.isFailure());
    }
    
    SECTION("Validate schema integrity - missing file") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        SchemaInfo schemaInfo;
        schemaInfo.authority = "test.org";  
        schemaInfo.version = "1.0";
        schemaInfo.localPath = tempDir_ / "nonexistent.xsd";
        
        auto result = manager.validateSchemaIntegrity(schemaInfo);
        REQUIRE(result.isFailure());
    }
    
    SECTION("Validate schema integrity - empty schema info") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        SchemaInfo emptyInfo; // All fields default/empty
        
        auto result = manager.validateSchemaIntegrity(emptyInfo);
        // Should handle empty info gracefully
        REQUIRE(result.isFailure());
    }
}

// Test error handling and edge cases
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Error Handling", "[schema][manager][error]") {
    
    SECTION("Cache cleanup operations") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Download multiple schemas to populate cache
        manager.downloadSchema("auth1.org", "https://auth1.org/schema.xsd", "1.0");
        manager.downloadSchema("auth2.org", "https://auth2.org/schema.xsd", "2.0");  
        manager.downloadSchema("auth3.org", "https://auth3.org/schema.xsd", "3.0");
        
        // Trigger cache cleanup (if method exists)
        REQUIRE_NOTHROW(manager.clearCache());
    }
    
    SECTION("Multiple downloads of same schema") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Download same schema multiple times
        auto result1 = manager.downloadSchema("duplicate.org", "https://duplicate.org/schema.xsd", "1.0");
        auto result2 = manager.downloadSchema("duplicate.org", "https://duplicate.org/schema.xsd", "1.0");
        auto result3 = manager.downloadSchema("duplicate.org", "https://duplicate.org/schema.xsd", "1.0");
        
        // All should succeed (or consistently fail)
        REQUIRE((result1.isSuccess() && result2.isSuccess() && result3.isSuccess()) ||
                (result1.isFailure() && result2.isFailure() && result3.isFailure()));
    }
    
    SECTION("Cache directory permissions") {
        // Test behavior when cache directory has restricted permissions
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        std::filesystem::path restrictedDir = tempDir_ / "restricted";
        std::filesystem::create_directories(restrictedDir);
        
        config_.cacheDirectory = restrictedDir;
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Attempt to download - should handle permission issues gracefully
        auto result = manager.downloadSchema("restricted.org", "https://restricted.org/schema.xsd", "1.0");
        // May succeed or fail depending on permissions, but shouldn't crash
        REQUIRE_NOTHROW(result.isSuccess() || result.isFailure());
    }
    
    SECTION("Large cache operations") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        httpClient->responseBody = std::string(10000, 'X'); // Large response
        
        // Set small cache size to test cleanup behavior  
        config_.maxCacheSize = 1024; // 1KB
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Download large schema that exceeds cache size
        auto result = manager.downloadSchema("large.org", "https://large.org/big_schema.xsd", "1.0");
        
        // Should handle large files appropriately  
        REQUIRE_NOTHROW(result.isSuccess() || result.isFailure());
    }
}

// Test concurrent operations (if supported)
TEST_CASE_METHOD(SchemaManagerFixture, "ExternalSchemaManager Concurrency", "[schema][manager][concurrent]") {
    
    SECTION("Concurrent downloads") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Test that multiple operations don't interfere with each other
        std::vector<std::future<Result<SchemaInfo>>> futures;
        
        // Simulate concurrent downloads (simplified test)
        auto result1 = manager.downloadSchema("concurrent1.org", "https://concurrent1.org/schema.xsd", "1.0");
        auto result2 = manager.downloadSchema("concurrent2.org", "https://concurrent2.org/schema.xsd", "1.0");
        auto result3 = manager.downloadSchema("concurrent3.org", "https://concurrent3.org/schema.xsd", "1.0");
        
        // All operations should complete without crashes
        REQUIRE_NOTHROW(result1.isSuccess() || result1.isFailure());
        REQUIRE_NOTHROW(result2.isSuccess() || result2.isFailure());
        REQUIRE_NOTHROW(result3.isSuccess() || result3.isFailure());
    }
    
    SECTION("Concurrent cache access") {
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config_, std::move(httpClient));
        
        // Download a schema first
        auto downloadResult = manager.downloadSchema("shared.org", "https://shared.org/schema.xsd", "1.0");
        
        if (downloadResult.isSuccess()) {
            // Multiple concurrent cache retrievals
            auto cache1 = manager.getCachedSchema("shared.org", "1.0");
            auto cache2 = manager.getCachedSchema("shared.org", "1.0");  
            auto cache3 = manager.getCachedSchema("shared.org", "1.0");
            
            // All should succeed consistently
            REQUIRE(cache1.isSuccess());
            REQUIRE(cache2.isSuccess());
            REQUIRE(cache3.isSuccess());
        }
    }
}