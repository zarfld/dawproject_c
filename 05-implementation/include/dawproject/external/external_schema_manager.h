/**
 * @file external_schema_manager.h
 * @brief External Schema Management Interface (DES-C-EXT-001)
 * 
 * Manages external schema downloads, caching, and validation against external authorities
 * Following IEEE 1016-2009 design specifications and ISO/IEC/IEEE 12207:2017
 * 
 * Traceability: DES-C-EXT-001 -> Implementation -> TEST-EXT-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 * XP Practices: TDD, Simple Design, YAGNI
 */

#pragma once

#include <dawproject/core/result.h>
#include <string>
#include <memory>
#include <filesystem>
#include <chrono>
#include <unordered_map>

namespace dawproject {

/**
 * @brief Schema information metadata
 * 
 * Contains essential information about cached external schemas
 * Supports external authority compliance tracking
 */
struct SchemaInfo {
    std::string version;                           ///< Schema version (e.g., "1.0.0")
    std::string url;                              ///< Original download URL
    std::string checksum;                         ///< SHA-256 checksum for integrity
    std::chrono::system_clock::time_point lastModified;  ///< Last modification time
    std::filesystem::path localPath;              ///< Local cache file path
    std::string authority;                        ///< External authority source
    std::string contentType;                      ///< Content type (e.g., "application/xml")
    bool isValid{true};                           ///< Validation status
};

/**
 * @brief HTTP client interface for external schema downloads
 * 
 * Abstract interface to allow different HTTP implementations
 * Supports external authority communication requirements
 */
class HTTPClient {
public:
    /**
     * @brief HTTP response structure
     */
    struct Response {
        int statusCode{0};                        ///< HTTP status code
        std::string body;                         ///< Response body content
        std::unordered_map<std::string, std::string> headers;  ///< Response headers
        std::string errorMessage;                 ///< Error description if failed
    };

    virtual ~HTTPClient() = default;

    /**
     * @brief Perform HTTP GET request
     * @param url Target URL
     * @param timeout Timeout in milliseconds
     * @return Result containing HTTP response or error info
     */
    virtual Result<Response> get(
        const std::string& url,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(30000)
    ) = 0;

    /**
     * @brief Check if URL is accessible
     * @param url Target URL to check
     * @return Result indicating accessibility
     */
    virtual Result<void> checkConnectivity(const std::string& url) = 0;
};

/**
 * @brief External Schema Manager Implementation
 * 
 * Manages downloading, caching, and serving external schemas for validation
 * Core component for external authority compliance (DES-C-EXT-001)
 */
class ExternalSchemaManager {
public:
    /**
     * @brief Configuration for schema manager
     */
    struct Config {
        std::filesystem::path cacheDirectory{"./.schema_cache"};  ///< Local cache directory
        std::chrono::hours cacheTTL{24};                          ///< Cache time-to-live
        bool enableOfflineMode{true};                             ///< Allow cached schemas when offline
        size_t maxCacheSize{100 * 1024 * 1024};                 ///< Max cache size (100MB)
        std::chrono::milliseconds networkTimeout{30000};         ///< Network timeout
        bool validateChecksums{true};                            ///< Enable integrity checking
    };

    /**
     * @brief Constructor with configuration and HTTP client
     * @param config Manager configuration
     * @param httpClient HTTP client implementation for downloads
     */
    ExternalSchemaManager(Config config, std::unique_ptr<HTTPClient> httpClient);

    /**
     * @brief Destructor
     */
    ~ExternalSchemaManager();

    /**
     * @brief Download and cache external schema
     * @param authority External authority (e.g., "dawproject.org")
     * @param schemaUrl URL to schema file
     * @param version Schema version identifier
     * @return Result containing schema info or error
     */
    Result<SchemaInfo> downloadSchema(
        const std::string& authority,
        const std::string& schemaUrl,
        const std::string& version
    );

    /**
     * @brief Get cached schema by authority and version
     * @param authority External authority identifier
     * @param version Schema version
     * @return Result containing schema info or error
     */
    Result<SchemaInfo> getCachedSchema(
        const std::string& authority,
        const std::string& version
    ) const;

    /**
     * @brief Get local path to cached schema file
     * @param authority External authority identifier
     * @param version Schema version
     * @return Result containing filesystem path or error
     */
    Result<std::filesystem::path> getSchemaPath(
        const std::string& authority,
        const std::string& version
    ) const;

    /**
     * @brief Validate schema integrity using checksums
     * @param schemaInfo Schema information to validate
     * @return Result indicating validation success or error
     */
    Result<void> validateSchemaIntegrity(const SchemaInfo& schemaInfo) const;

    /**
     * @brief Clean expired schemas from cache
     * @return Result containing number of cleaned files or error
     */
    Result<size_t> cleanExpiredSchemas();

    /**
     * @brief Get cache statistics
     * @return Result containing cache usage information or error
     */
    Result<std::unordered_map<std::string, size_t>> getCacheStatistics() const;

    /**
     * @brief Check if schema is available (cached or downloadable)
     * @param authority External authority identifier
     * @param schemaUrl URL to check
     * @param version Schema version
     * @return Result indicating availability
     */
    Result<bool> isSchemaAvailable(
        const std::string& authority,
        const std::string& schemaUrl,
        const std::string& version
    ) const;

    /**
     * @brief List all cached schemas
     * @return Result containing list of cached schema info or error
     */
    Result<std::vector<SchemaInfo>> listCachedSchemas() const;

    /**
     * @brief Remove specific schema from cache
     * @param authority External authority identifier
     * @param version Schema version
     * @return Result indicating removal success or error
     */
    Result<void> removeSchema(
        const std::string& authority,
        const std::string& version
    );

    /**
     * @brief Configure offline mode
     * @param enabled Enable/disable offline mode
     */
    void setOfflineMode(bool enabled);

    /**
     * @brief Get current configuration
     * @return Current manager configuration
     */
    const Config& getConfig() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  ///< PIMPL pattern for implementation hiding
};

/**
 * @brief Default HTTP client factory
 * @return Unique pointer to default HTTP client implementation
 */
std::unique_ptr<HTTPClient> createDefaultHTTPClient();

} // namespace dawproject