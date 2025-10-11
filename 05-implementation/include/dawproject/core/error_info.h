#pragma once

/**
 * @file error_info.h
 * @brief Comprehensive Error Information (DES-I-CORE-001)
 * 
 * Provides structured error information for all library operations
 * as specified in Phase 04 detailed design.
 * 
 * Traceability: DES-I-CORE-001, REQ-NF-R-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <chrono>
#include <memory>
#include <map>

namespace dawproject {

/**
 * @brief Comprehensive error information for all library operations
 * 
 * Provides structured error information including:
 * - Error categorization and severity
 * - Contextual information and suggested actions
 * - External authority compliance context
 * - Performance and debugging information
 */
struct ErrorInfo {
    /**
     * @brief Error category for systematic error handling
     */
    enum class Category {
        // Network and external authority errors
        NetworkError,           // HTTP, connectivity issues
        ExternalAuthorityError, // Schema access, compliance violations
        
        // Validation and parsing errors  
        ValidationError,        // XSD validation failures
        ParseError,            // XML/JSON parsing issues
        
        // File system and I/O errors
        FileSystemError,       // File access, permissions
        CacheError,           // Cache management issues
        
        // Configuration and setup errors
        ConfigurationError,    // Invalid configuration
        DependencyError,      // Missing dependencies (libxml2, libcurl)
        
        // Runtime and system errors
        MemoryError,          // Allocation failures
        TimeoutError,         // Operation timeouts
        ConcurrencyError,     // Thread safety violations
        
        // Business logic errors
        InvalidOperation,     // Unsupported operations
        InvalidData,         // Data integrity issues
        
        // Unknown or system errors
        UnknownError         // Catch-all for unexpected issues
    };
    
    /**
     * @brief Error severity for prioritization and handling
     */
    enum class Severity {
        Info,       // Informational - operation can continue
        Warning,    // Warning - operation succeeded with issues
        Error,      // Error - operation failed but recoverable
        Critical,   // Critical - operation failed, system integrity at risk
        Fatal       // Fatal - operation failed, immediate shutdown required
    };
    
    // Core error information
    Category category = Category::UnknownError;
    Severity severity = Severity::Error;
    std::string code;           // Machine-readable error code (e.g., "EXT_SCHEMA_001")
    std::string message;        // Human-readable error description
    std::string details;        // Detailed technical information
    
    // Contextual information
    std::optional<std::string> operation;      // Operation that failed
    std::optional<std::string> component;      // Component where error occurred
    std::optional<std::filesystem::path> filePath; // Related file path
    std::optional<std::string> url;           // Related URL (for network errors)
    
    // External authority context
    std::optional<std::string> externalSource; // External authority source
    std::optional<std::string> schemaVersion;  // Schema version related to error
    bool externalAuthorityRelated = false;     // Error related to external authority
    
    // Recovery and action information
    std::vector<std::string> suggestedActions; // User-actionable recovery steps
    std::vector<std::string> technicalActions; // Developer-actionable steps
    bool isRecoverable = true;                 // Whether error is recoverable
    
    // Debugging and performance information
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now(); // When error occurred
    std::optional<std::chrono::milliseconds> duration; // Operation duration
    std::optional<size_t> memoryUsage;        // Memory usage at time of error
    std::optional<std::string> stackTrace;    // Stack trace (debug builds)
    
    // Chained error support
    std::shared_ptr<ErrorInfo> cause;         // Underlying cause (if any)
    std::vector<ErrorInfo> relatedErrors;     // Related errors
    
    // Default constructor
    ErrorInfo() = default;
    
    // Constructor with basic info
    ErrorInfo(Category cat, Severity sev, std::string code, std::string message)
        : category(cat), severity(sev), code(std::move(code)), message(std::move(message)) {}
    
    // Factory methods for common error types
    static ErrorInfo networkError(
        const std::string& message,
        const std::string& url = "",
        const std::string& details = ""
    );
    
    static ErrorInfo externalAuthorityError(
        const std::string& message,
        const std::string& externalSource,
        const std::string& details = ""
    );
    
    static ErrorInfo validationError(
        const std::string& message,
        const std::filesystem::path& filePath = {},
        const std::string& details = ""
    );
    
    static ErrorInfo parseError(
        const std::string& message,
        const std::filesystem::path& filePath = {},
        const std::string& details = ""
    );
    
    static ErrorInfo fileSystemError(
        const std::string& message,
        const std::filesystem::path& filePath = {},
        const std::string& details = ""
    );
    
    static ErrorInfo configurationError(
        const std::string& message,
        const std::string& component,
        const std::vector<std::string>& suggestedActions = {}
    );
    
    static ErrorInfo dependencyError(
        const std::string& dependency,
        const std::string& details = "",
        const std::vector<std::string>& installInstructions = {}
    );
    
    static ErrorInfo timeoutError(
        const std::string& operation,
        std::chrono::milliseconds timeout,
        const std::string& details = ""
    );
    
    static ErrorInfo memoryError(
        const std::string& message,
        size_t requestedSize = 0,
        const std::string& details = ""
    );
    
    static ErrorInfo invalidOperationError(
        const std::string& operation,
        const std::string& reason,
        const std::string& details = ""
    );
    
    static ErrorInfo invalidDataError(
        const std::string& message,
        const std::string& dataContext = "",
        const std::string& details = ""
    );
    
    // Utility methods
    std::string toString() const;
    std::map<std::string, std::string> toMap() const;
    
    // Category and severity string conversion
    static std::string categoryToString(Category category);
    static std::string severityToString(Severity severity);
};

} // namespace dawproject