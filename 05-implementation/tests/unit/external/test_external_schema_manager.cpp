/**
 * @file test_external_schema_manager.cpp
 * @brief Unit Tests for ExternalSchemaManager (TDD Implementation)
 * 
 * Tests for DES-C-EXT-001 ExternalSchemaManager implementation
 * Following Red-Green-Refactor TDD methodology
 * 
 * Traceability: DES-C-EXT-001 -> TEST-EXT-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <catch2/catch_test_macros.hpp>
#include <dawproject/external/external_schema_manager.h>
#include <dawproject/core/error_info.h>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace dawproject;

/**
 * @brief Mock HTTP client for testing
 */
class MockHTTPClient : public HTTPClient {
public:
    bool shouldFailDownload = false;
    bool shouldFailConnectivity = false;
    int statusCode = 200;
    std::string mockContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<schema xmlns="http://www.w3.org/2001/XMLSchema">
  <element name="TestElement" type="string"/>
</schema>)";
    
    Result<Response> get(
        const std::string& url,
        std::chrono::milliseconds timeout) override {
        
        (void)timeout; // Suppress unused parameter warning
        
        if (shouldFailDownload) {
            return Result<Response>::error(
                ErrorInfo::networkError("Simulated download failure", url)
            );
        }
        
        Response response;
        response.statusCode = statusCode;
        response.body = mockContent;
        response.headers["content-type"] = "application/xml";
        response.headers["content-length"] = std::to_string(mockContent.size());
        
        return Result<Response>::success(std::move(response));
    }
    
    Result<void> checkConnectivity(const std::string& url) override {
        if (shouldFailConnectivity) {
            return Result<void>::error(
                ErrorInfo::networkError("Simulated connectivity failure", url)
            );
        }
        
        return Result<void>::success();
    }
};

TEST_CASE("ExternalSchemaManager - Construction and Configuration", "[external-schema][construction]") {
    SECTION("Valid construction with default config") {
        ExternalSchemaManager::Config config;
        auto httpClient = std::make_unique<MockHTTPClient>();
        
        ExternalSchemaManager manager(config, std::move(httpClient));
        
        REQUIRE(manager.getConfig().cacheDirectory == "./.schema_cache");
        REQUIRE(manager.getConfig().cacheTTL == std::chrono::hours(24));
        REQUIRE(manager.getConfig().enableOfflineMode == true);
    }
    
    SECTION("Construction with custom config") {
        ExternalSchemaManager::Config config;
        config.cacheDirectory = "./test_cache";
        config.cacheTTL = std::chrono::hours(12);
        config.enableOfflineMode = false;
        config.maxCacheSize = 50 * 1024 * 1024;
        
        auto httpClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(httpClient));
        
        REQUIRE(manager.getConfig().cacheDirectory == "./test_cache");
        REQUIRE(manager.getConfig().cacheTTL == std::chrono::hours(12));
        REQUIRE(manager.getConfig().enableOfflineMode == false);
        REQUIRE(manager.getConfig().maxCacheSize == 50 * 1024 * 1024);
    }
}

TEST_CASE("ExternalSchemaManager - Schema Download", "[external-schema][download]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_download";
    
    SECTION("Successful schema download") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "dawproject.org",
            "https://dawproject.org/schema/v1.0.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isSuccess());
        
        auto schema = result.value();
        REQUIRE(schema.authority == "dawproject.org");
        REQUIRE(schema.version == "1.0.0");
        REQUIRE(schema.url == "https://dawproject.org/schema/v1.0.xsd");
        REQUIRE(schema.contentType == "application/xml");
        REQUIRE(schema.isValid == true);
        REQUIRE(!schema.checksum.empty());
        REQUIRE(std::filesystem::exists(schema.localPath));
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_download");
    }
    
    SECTION("Download with empty authority fails") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "",  // Empty authority
            "https://example.com/schema.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::InvalidData);
    }
    
    SECTION("Download with empty URL fails") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "example.org",
            "",  // Empty URL
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::InvalidData);
    }
    
    SECTION("Download with empty version fails") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "example.org",
            "https://example.com/schema.xsd",
            ""  // Empty version
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::InvalidData);
    }
    
    SECTION("Network failure during download") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        mockClient->shouldFailDownload = true;
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "example.org",
            "https://unreachable.example.com/schema.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ExternalAuthorityError);
        REQUIRE(result.error().externalAuthorityRelated == true);
    }
    
    SECTION("HTTP error status code") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        mockClient->statusCode = 404;
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "example.org",
            "https://example.com/notfound.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ExternalAuthorityError);
        REQUIRE(result.error().message.find("HTTP error") != std::string::npos);
    }
    
    SECTION("Empty response body") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        mockClient->mockContent = "";  // Empty content
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "example.org",
            "https://example.com/empty.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ExternalAuthorityError);
        REQUIRE(result.error().message.find("Empty schema content") != std::string::npos);
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_download");
}

TEST_CASE("ExternalSchemaManager - Cache Operations", "[external-schema][cache]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_ops";
    
    SECTION("Cache retrieval after download") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // First download
        auto downloadResult = manager.downloadSchema(
            "example.org",
            "https://example.com/schema.xsd",
            "2.0.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Retrieve from cache
        auto cacheResult = manager.getCachedSchema("example.org", "2.0.0");
        REQUIRE(cacheResult.isSuccess());
        
        auto cached = cacheResult.value();
        REQUIRE(cached.authority == "example.org");
        REQUIRE(cached.version == "2.0.0");
        REQUIRE(cached.isValid == true);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_ops");
    }
    
    SECTION("Cache miss for non-existent schema") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.getCachedSchema("nonexistent.org", "1.0.0");
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ConfigurationError);
    }
    
    SECTION("Get schema path for cached schema") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema first
        auto downloadResult = manager.downloadSchema(
            "test.org",
            "https://test.org/schema.xsd",
            "1.5.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Get path
        auto pathResult = manager.getSchemaPath("test.org", "1.5.0");
        REQUIRE(pathResult.isSuccess());
        REQUIRE(std::filesystem::exists(pathResult.value()));
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_ops");
    }
    
    SECTION("List cached schemas") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Initially empty
        auto listResult = manager.listCachedSchemas();
        REQUIRE(listResult.isSuccess());
        REQUIRE(listResult.value().empty());
        
        // Download two schemas
        manager.downloadSchema("auth1.org", "https://auth1.org/v1.xsd", "1.0.0");
        manager.downloadSchema("auth2.org", "https://auth2.org/v2.xsd", "2.0.0");
        
        // List should contain both
        listResult = manager.listCachedSchemas();
        REQUIRE(listResult.isSuccess());
        REQUIRE(listResult.value().size() == 2);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_ops");
    }
    
    SECTION("Remove schema from cache") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema
        auto downloadResult = manager.downloadSchema(
            "remove-test.org",
            "https://remove-test.org/schema.xsd",
            "1.0.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Verify it's cached
        auto cacheResult = manager.getCachedSchema("remove-test.org", "1.0.0");
        REQUIRE(cacheResult.isSuccess());
        
        // Remove it
        auto removeResult = manager.removeSchema("remove-test.org", "1.0.0");
        REQUIRE(removeResult.isSuccess());
        
        // Verify it's gone
        cacheResult = manager.getCachedSchema("remove-test.org", "1.0.0");
        REQUIRE(cacheResult.isError());
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_ops");
    }
    
    SECTION("Remove non-existent schema fails") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.removeSchema("nonexistent.org", "1.0.0");
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ConfigurationError);
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_ops");
}

TEST_CASE("ExternalSchemaManager - Cache Management", "[external-schema][cache-management]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_mgmt";
    config.cacheTTL = std::chrono::hours(0);  // Immediate expiry for testing
    
    SECTION("Cache statistics") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Initially empty
        auto statsResult = manager.getCacheStatistics();
        REQUIRE(statsResult.isSuccess());
        
        auto stats = statsResult.value();
        REQUIRE(stats["total_schemas"] == 0);
        REQUIRE(stats["cache_size_bytes"] == 0);
        
        // Add schema
        manager.downloadSchema("stats-test.org", "https://stats-test.org/schema.xsd", "1.0.0");
        
        // Check updated stats
        statsResult = manager.getCacheStatistics();
        REQUIRE(statsResult.isSuccess());
        
        stats = statsResult.value();
        REQUIRE(stats["total_schemas"] == 1);
        REQUIRE(stats["cache_size_bytes"] > 0);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_mgmt");
    }
    
    SECTION("Clean expired schemas") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema
        manager.downloadSchema("expire-test.org", "https://expire-test.org/schema.xsd", "1.0.0");
        
        // Wait for expiration
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        // Clean expired
        auto cleanResult = manager.cleanExpiredSchemas();
        REQUIRE(cleanResult.isSuccess());
        REQUIRE(cleanResult.value() == 1);
        
        // Verify schema is gone
        auto cacheResult = manager.getCachedSchema("expire-test.org", "1.0.0");
        REQUIRE(cacheResult.isError());
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_mgmt");
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_mgmt");
}

TEST_CASE("ExternalSchemaManager - Offline Mode", "[external-schema][offline]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_offline";
    
    SECTION("Offline mode with cached schema") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema first
        auto downloadResult = manager.downloadSchema(
            "offline-test.org",
            "https://offline-test.org/schema.xsd",
            "1.0.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        // Enable offline mode
        manager.setOfflineMode(true);
        
        // Should still be able to get cached schema
        auto result = manager.getCachedSchema("offline-test.org", "1.0.0");
        REQUIRE(result.isSuccess());
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_offline");
    }
    
    SECTION("Schema availability check in offline mode") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema
        manager.downloadSchema("avail-test.org", "https://avail-test.org/schema.xsd", "1.0.0");
        
        // Enable offline mode
        manager.setOfflineMode(true);
        
        // Cached schema should be available
        auto availResult = manager.isSchemaAvailable(
            "avail-test.org",
            "https://avail-test.org/schema.xsd",
            "1.0.0"
        );
        REQUIRE(availResult.isSuccess());
        REQUIRE(availResult.value() == true);
        
        // Non-cached schema should not be available in offline mode
        availResult = manager.isSchemaAvailable(
            "other.org",
            "https://other.org/schema.xsd",
            "2.0.0"
        );
        REQUIRE(availResult.isSuccess());
        REQUIRE(availResult.value() == false);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_offline");
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_offline");
}

TEST_CASE("ExternalSchemaManager - Schema Integrity", "[external-schema][integrity]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_integrity";
    config.validateChecksums = true;
    
    SECTION("Valid schema integrity check") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Download schema
        auto downloadResult = manager.downloadSchema(
            "integrity-test.org",
            "https://integrity-test.org/schema.xsd",
            "1.0.0"
        );
        REQUIRE(downloadResult.isSuccess());
        
        auto schema = downloadResult.value();
        
        // Validate integrity
        auto validationResult = manager.validateSchemaIntegrity(schema);
        REQUIRE(validationResult.isSuccess());
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_integrity");
    }
    
    SECTION("Integrity check with missing file") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        // Create fake schema info with non-existent file
        SchemaInfo fakeSchema;
        fakeSchema.localPath = "./non-existent-file.xsd";
        fakeSchema.checksum = "fake-checksum";
        
        auto validationResult = manager.validateSchemaIntegrity(fakeSchema);
        REQUIRE(validationResult.isError());
        REQUIRE(validationResult.error().category == ErrorInfo::Category::FileSystemError);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_integrity");
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_integrity");
}

// External Authority Integration Tests
TEST_CASE("ExternalSchemaManager - External Authority Integration", "[external-schema][external-authority]") {
    ExternalSchemaManager::Config config;
    config.cacheDirectory = "./test_cache_authority";
    
    SECTION("Download failure generates external authority error") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        mockClient->shouldFailDownload = true;
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto result = manager.downloadSchema(
            "dawproject.org",
            "https://dawproject.org/schema/v1.0.xsd",
            "1.0.0"
        );
        
        REQUIRE(result.isError());
        REQUIRE(result.error().category == ErrorInfo::Category::ExternalAuthorityError);
        REQUIRE(result.error().externalAuthorityRelated == true);
        REQUIRE(result.error().externalSource.has_value());
        REQUIRE(result.error().externalSource.value() == "dawproject.org");
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_authority");
    }
    
    SECTION("Connectivity check failure") {
        auto mockClient = std::make_unique<MockHTTPClient>();
        mockClient->shouldFailConnectivity = true;
        ExternalSchemaManager manager(config, std::move(mockClient));
        
        auto availResult = manager.isSchemaAvailable(
            "unreachable.org",
            "https://unreachable.org/schema.xsd",
            "1.0.0"
        );
        
        REQUIRE(availResult.isSuccess());
        REQUIRE(availResult.value() == false);
        
        // Cleanup
        std::filesystem::remove_all("./test_cache_authority");
    }
    
    // Cleanup
    std::filesystem::remove_all("./test_cache_authority");
}

TEST_CASE("ExternalSchemaManager - Factory Functions", "[external-schema][factory]") {
    SECTION("Default HTTP client creation") {
        auto client = createDefaultHTTPClient();
        
        REQUIRE(client != nullptr);
        
        // Test basic functionality
        auto result = client->get("https://dawproject.org/test", std::chrono::milliseconds(1000));
        // Should not crash - specific behavior depends on implementation
        REQUIRE((result.isSuccess() || result.isError()));
    }
}