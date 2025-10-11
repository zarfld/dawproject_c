/**
 * @file external_schema_manager.h
 * @brief External Schema Management Interface (Infrastructure Layer)
 * 
 * Manages external schema downloads, caching, and validation against external authorities
 * Following IEEE 1016-2009 design specifications and ISO/IEC/IEEE 12207:2017
 * 
 * Architecture: Infrastructure Layer - Schema Management
 * - Handles external schema download and caching
 * - Provides schema integrity validation
 * - Manages external authority compliance
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
namespace infrastructure {
namespace schema_management {

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
        bool success{false};                      ///< Operation success flag
    };
    
    virtual ~HTTPClient() = default;
    
    /**
     * @brief Download content from URL
     * 
     * @param url Target URL for download
     * @param timeout Connection timeout in seconds
     * @return Response structure with download results
     */
    virtual Response get(const std::string& url, int timeout = 30) = 0;
};

/**
 * @brief External Schema Manager
 * 
 * Manages downloading, caching, and validation of external XML schemas
 * Supports compliance with external authorities per requirements
 * 
 * Features:
 * - Schema caching with integrity verification
 * - External authority compliance tracking
 * - Offline mode support
 * - Cache expiration and cleanup
 * - Performance optimization through caching
 */
class ExternalSchemaManager {
public:
    /**
     * @brief Configuration for schema management
     */
    struct Config {
        std::filesystem::path cacheDirectory{"schema_cache"};  ///< Local cache directory
        std::chrono::hours cacheExpiration{24};              ///< Cache expiration time
        bool enableOfflineMode{false};                       ///< Offline operation mode
        size_t maxCacheSize{100 * 1024 * 1024};             ///< Max cache size (100MB)
        int downloadTimeout{30};                             ///< Download timeout (seconds)
        bool verifyChecksums{true};                          ///< Enable checksum verification
        std::string userAgent{"DAWProject-Validator/1.0"};   ///< HTTP User-Agent string
    };
    
    /**
     * @brief Constructor with configuration and HTTP client
     * 
     * @param config Schema manager configuration
     * @param httpClient HTTP client implementation (optional, uses default if null)
     */
    explicit ExternalSchemaManager(Config config = {}, std::unique_ptr<HTTPClient> httpClient = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ExternalSchemaManager();
    
    // Non-copyable but movable
    ExternalSchemaManager(const ExternalSchemaManager&) = delete;
    ExternalSchemaManager& operator=(const ExternalSchemaManager&) = delete;
    ExternalSchemaManager(ExternalSchemaManager&&) = default;
    ExternalSchemaManager& operator=(ExternalSchemaManager&&) = default;
    
    /**
     * @brief Download schema from external authority
     * 
     * Downloads and caches external schema with integrity verification
     * Supports DAWProject.org schema authority compliance
     * 
     * @param url Schema URL to download
     * @param version Expected schema version (optional)
     * @return SchemaInfo with download results
     */
    Result<SchemaInfo> downloadSchema(const std::string& url, const std::string& version = "");
    
    /**
     * @brief Get cached schema information
     * 
     * @param url Schema URL to lookup
     * @return SchemaInfo if found in cache
     */
    Result<SchemaInfo> getCachedSchema(const std::string& url) const;
    
    /**
     * @brief Get local path to schema file
     * 
     * @param url Schema URL to lookup
     * @return Local filesystem path to cached schema
     */
    Result<std::filesystem::path> getSchemaPath(const std::string& url) const;
    
    /**
     * @brief Validate schema integrity
     * 
     * @param schemaInfo Schema information to validate
     * @return Success if schema passes integrity checks
     */
    Result<void> validateSchemaIntegrity(const SchemaInfo& schemaInfo) const;
    
    /**
     * @brief Clean expired schemas from cache
     * 
     * @return Number of schemas removed
     */
    Result<size_t> cleanExpiredSchemas();
    
    /**
     * @brief Get cache statistics
     * 
     * @return Map of cache statistics (size, count, etc.)
     */
    Result<std::unordered_map<std::string, size_t>> getCacheStatistics() const;
    
    /**
     * @brief Check if schema is available (cached or downloadable)
     * 
     * @param url Schema URL to check
     * @return True if schema is available
     */
    Result<bool> isSchemaAvailable(const std::string& url) const;
    
    /**
     * @brief List all cached schemas
     * 
     * @return Vector of cached schema information
     */
    Result<std::vector<SchemaInfo>> listCachedSchemas() const;
    
    /**
     * @brief Remove schema from cache
     * 
     * @param url Schema URL to remove
     * @return Success if removed or didn't exist
     */
    Result<void> removeSchema(const std::string& url);
    
    /**
     * @brief Enable/disable offline mode
     * 
     * In offline mode, only cached schemas are used
     * 
     * @param enabled True to enable offline mode
     */
    void setOfflineMode(bool enabled);
    
    /**
     * @brief Get current configuration
     * 
     * @return Current configuration settings
     */
    const Config& getConfig() const;

private:
    // Forward declaration for PIMPL pattern
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace schema_management
} // namespace infrastructure
} // namespace dawproject