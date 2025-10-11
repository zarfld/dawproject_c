#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <filesystem>
#include <memory>

namespace dawproject {
namespace domain {
namespace validation {

/**
 * @brief Type of external schema for validation
 */
enum class SchemaType {
    ProjectSchema,   // Project.xsd from external authority
    MetaDataSchema   // MetaData.xsd from external authority
};

/**
 * @brief Detailed validation error information (Domain Entity)
 */
struct ValidationError {
    enum class Severity {
        Error,      // XSD validation error - document invalid
        Warning,    // XSD warning - document valid but issues detected
        Info        // Informational message
    };
    
    Severity severity;
    std::string message;                    // Human-readable error description
    std::optional<int> lineNumber;          // Line number in XML (if available)
    std::optional<int> columnNumber;        // Column number in XML (if available)
    std::optional<std::string> xpath;       // XPath to problematic element (if available)
    std::optional<std::string> xsdRule;     // XSD rule that was violated (if available)
    std::chrono::system_clock::time_point validationTime; // When error was detected
};

/**
 * @brief Comprehensive validation report (Domain Entity)
 */
struct ValidationReport {
    // Validation results
    bool isValid = false;
    std::vector<ValidationError> errors;
    std::size_t totalErrorCount = 0;
    std::size_t totalWarningCount = 0;
    
    // Schema information
    SchemaType schemaType;
    std::filesystem::path schemaPath;
    std::string schemaChecksum;
    bool externalAuthorityCompliant = false;
    std::string externalSchemaSource;
    
    // Performance metrics
    std::chrono::milliseconds validationTime{0};
    std::chrono::milliseconds schemaLoadTime{0};
    std::chrono::milliseconds schemaRetrievalTime{0};
    
    // Document statistics
    std::size_t documentSize = 0;
    std::size_t elementCount = 0;
    std::size_t attributeCount = 0;
    
    // Timing information
    std::chrono::system_clock::time_point validationStartTime;
};

/**
 * @brief Validation request for batch operations (Domain Value Object)
 */
struct ValidationRequest {
    std::string xmlContent;
    SchemaType schemaType;
    std::string requestId;                      // For correlation in batch results
    std::optional<std::string> sourceLocation;  // File path or URI for context
};

/**
 * @brief Configuration for external validation operations (Domain Value Object)
 */
struct ExternalValidationConfig {
    bool enableXMLSecurity = true;          // Prevent XXE attacks
    std::chrono::seconds schemaTimeout{30}; // Timeout for schema retrieval
    bool enableCaching = true;              // Enable schema caching
    bool enableDetailedErrors = true;       // Include detailed error context
    std::optional<std::string> userAgent;   // User agent for HTTP requests
};

/**
 * @brief Schema availability information (Domain Entity)
 */
struct SchemaAvailabilityInfo {
    bool projectSchemaAvailable = false;
    bool metaDataSchemaAvailable = false;
    std::chrono::system_clock::time_point lastChecked;
    std::string projectSchemaUrl;
    std::string metaDataSchemaUrl;
    std::optional<std::string> errorMessage; // If schemas unavailable
};

/**
 * @brief Information about the validation engine (Domain Entity)
 */
struct ValidationEngineInfo {
    std::string engineVersion;
    std::string libxml2Version;
    bool externalAuthoritySupport = false;
    std::vector<std::string> supportedSchemaTypes;
    std::chrono::system_clock::time_point buildTimestamp;
};

} // namespace validation
} // namespace domain
} // namespace dawproject