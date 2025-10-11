/**
 * @file error_info.cpp
 * @brief ErrorInfo Implementation (DES-I-CORE-001)
 * 
 * Implements factory methods and utility functions for ErrorInfo
 * as specified in Phase 04 detailed design.
 * 
 * Traceability: DES-I-CORE-001, REQ-NF-R-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <dawproject/core/error_info.h>
#include <sstream>
#include <iomanip>
#include <atomic>

namespace dawproject {

// Static error code counter for unique error IDs
static std::atomic<size_t> g_errorCounter{1000};

static std::string generateErrorCode(const std::string& prefix) {
    auto id = g_errorCounter.fetch_add(1);
    std::ostringstream oss;
    oss << prefix << "_" << std::setfill('0') << std::setw(3) << id;
    return oss.str();
}

ErrorInfo ErrorInfo::networkError(
    const std::string& message,
    const std::string& url,
    const std::string& details) {
    
    ErrorInfo error(Category::NetworkError, Severity::Error,
                   generateErrorCode("NET"), message);
    error.details = details;
    error.url = url;
    error.operation = "network_operation";
    
    // Always provide basic network troubleshooting steps
    error.suggestedActions.push_back("Check network connectivity");
    if (!url.empty()) {
        error.suggestedActions.push_back("Verify URL is accessible: " + url);
    }
    error.suggestedActions.push_back("Check firewall and proxy settings");
    error.suggestedActions.push_back("Retry the operation after a brief delay");
    
    error.technicalActions.push_back("Review network logs");
    error.technicalActions.push_back("Check DNS resolution");
    error.technicalActions.push_back("Verify SSL/TLS certificates");
    
    return error;
}

ErrorInfo ErrorInfo::externalAuthorityError(
    const std::string& message,
    const std::string& externalSource,
    const std::string& details) {
    
    ErrorInfo error(Category::ExternalAuthorityError, Severity::Critical,
                   generateErrorCode("EXT"), message);
    error.details = details;
    error.externalSource = externalSource;
    error.externalAuthorityRelated = true;
    error.operation = "external_authority_compliance";
    
    error.suggestedActions.push_back("Verify external authority source: " + externalSource);
    error.suggestedActions.push_back("Check external authority availability");
    error.suggestedActions.push_back("Update to latest external authority version");
    
    error.technicalActions.push_back("Review external authority integration logs");
    error.technicalActions.push_back("Validate external authority configuration");
    error.technicalActions.push_back("Check external authority API compatibility");
    
    return error;
}

ErrorInfo ErrorInfo::validationError(
    const std::string& message,
    const std::filesystem::path& filePath,
    const std::string& details) {
    
    ErrorInfo error(Category::ValidationError, Severity::Error,
                   generateErrorCode("VAL"), message);
    error.details = details;
    if (!filePath.empty()) {
        error.filePath = filePath;
    }
    error.operation = "validation";
    
    if (!filePath.empty()) {
        error.suggestedActions.push_back("Check file format: " + filePath.string());
        error.suggestedActions.push_back("Validate file against schema");
        error.suggestedActions.push_back("Verify file is not corrupted");
    } else {
        error.suggestedActions.push_back("Check validation rules");
        error.suggestedActions.push_back("Verify input data format");
        error.suggestedActions.push_back("Review validation criteria");
    }
    
    error.technicalActions.push_back("Review validation rules");
    error.technicalActions.push_back("Check schema compatibility");
    error.technicalActions.push_back("Examine validation error details");
    
    return error;
}

ErrorInfo ErrorInfo::parseError(
    const std::string& message,
    const std::filesystem::path& filePath,
    const std::string& details) {
    
    ErrorInfo error(Category::ParseError, Severity::Error,
                   generateErrorCode("PARSE"), message);
    error.details = details;
    error.filePath = filePath;
    error.operation = "parsing";
    
    if (!filePath.empty()) {
        error.suggestedActions.push_back("Check file encoding: " + filePath.string());
        error.suggestedActions.push_back("Verify file is valid XML/JSON");
        error.suggestedActions.push_back("Check for special characters or corrupted data");
    }
    
    error.technicalActions.push_back("Review parser configuration");
    error.technicalActions.push_back("Check character encoding settings");
    error.technicalActions.push_back("Validate file structure manually");
    
    return error;
}

ErrorInfo ErrorInfo::fileSystemError(
    const std::string& message,
    const std::filesystem::path& filePath,
    const std::string& details) {
    
    ErrorInfo error(Category::FileSystemError, Severity::Error,
                   generateErrorCode("FS"), message);
    error.details = details;
    error.filePath = filePath;
    error.operation = "file_system_operation";
    
    if (!filePath.empty()) {
        error.suggestedActions.push_back("Check file exists: " + filePath.string());
        error.suggestedActions.push_back("Verify file permissions");
        error.suggestedActions.push_back("Check available disk space");
    }
    
    error.technicalActions.push_back("Review file system permissions");
    error.technicalActions.push_back("Check disk space and quotas");
    error.technicalActions.push_back("Verify file system integrity");
    
    return error;
}

ErrorInfo ErrorInfo::configurationError(
    const std::string& message,
    const std::string& component,
    const std::vector<std::string>& suggestedActions) {
    
    ErrorInfo error(Category::ConfigurationError, Severity::Error,
                   generateErrorCode("CFG"), message);
    error.component = component;
    error.operation = "configuration";
    error.suggestedActions = suggestedActions;
    
    if (suggestedActions.empty()) {
        error.suggestedActions.push_back("Review configuration settings");
        error.suggestedActions.push_back("Check configuration file syntax");
        error.suggestedActions.push_back("Restore default configuration");
    }
    
    error.technicalActions.push_back("Validate configuration schema");
    error.technicalActions.push_back("Check environment variables");
    error.technicalActions.push_back("Review configuration documentation");
    
    return error;
}

ErrorInfo ErrorInfo::dependencyError(
    const std::string& dependency,
    const std::string& details,
    const std::vector<std::string>& installInstructions) {
    
    ErrorInfo error(Category::DependencyError, Severity::Critical,
                   generateErrorCode("DEP"), "Missing or incompatible dependency: " + dependency);
    error.details = details;
    error.operation = "dependency_check";
    
    error.suggestedActions.push_back("Install missing dependency: " + dependency);
    for (const auto& instruction : installInstructions) {
        error.suggestedActions.push_back(instruction);
    }
    if (installInstructions.empty()) {
        error.suggestedActions.push_back("Check system package manager");
        error.suggestedActions.push_back("Verify installation path");
    }
    
    error.technicalActions.push_back("Check library search paths");
    error.technicalActions.push_back("Verify dependency version compatibility");
    error.technicalActions.push_back("Review build configuration");
    
    return error;
}

ErrorInfo ErrorInfo::timeoutError(
    const std::string& operation,
    std::chrono::milliseconds timeout,
    const std::string& details) {
    
    ErrorInfo error(Category::TimeoutError, Severity::Error,
                   generateErrorCode("TIMEOUT"), "Operation timed out: " + operation);
    error.details = details;
    error.operation = operation;
    error.duration = timeout;
    
    std::string timeoutStr = std::to_string(timeout.count()) + "ms";
    error.suggestedActions.push_back("Increase timeout value (current: " + timeoutStr + ")");
    error.suggestedActions.push_back("Check system performance");
    error.suggestedActions.push_back("Verify network connectivity (if applicable)");
    
    error.technicalActions.push_back("Monitor system resources");
    error.technicalActions.push_back("Profile operation performance");
    error.technicalActions.push_back("Review timeout configuration");
    
    return error;
}

ErrorInfo ErrorInfo::memoryError(
    const std::string& message,
    size_t requestedSize,
    const std::string& details) {
    
    ErrorInfo error(Category::MemoryError, Severity::Critical,
                   generateErrorCode("MEM"), message);
    error.details = details;
    error.operation = "memory_allocation";
    
    if (requestedSize > 0) {
        error.memoryUsage = requestedSize;
        std::string sizeStr = std::to_string(requestedSize / 1024 / 1024) + "MB";
        error.suggestedActions.push_back("Reduce memory usage (requested: " + sizeStr + ")");
    }
    
    error.suggestedActions.push_back("Check available system memory");
    error.suggestedActions.push_back("Close other applications");
    error.suggestedActions.push_back("Consider processing in smaller chunks");
    
    error.technicalActions.push_back("Review memory allocation patterns");
    error.technicalActions.push_back("Check for memory leaks");
    error.technicalActions.push_back("Profile memory usage");
    
    return error;
}

ErrorInfo ErrorInfo::invalidOperationError(
    const std::string& operation,
    const std::string& reason,
    const std::string& details) {
    
    ErrorInfo error(Category::InvalidOperation, Severity::Error,
                   generateErrorCode("INVOP"), "Invalid operation: " + operation);
    error.details = details + " Reason: " + reason;
    error.operation = operation;
    
    error.suggestedActions.push_back("Check operation parameters");
    error.suggestedActions.push_back("Verify operation is supported");
    error.suggestedActions.push_back("Review operation sequence");
    
    error.technicalActions.push_back("Check API documentation");
    error.technicalActions.push_back("Verify operation preconditions");
    error.technicalActions.push_back("Review operation state machine");
    
    return error;
}

ErrorInfo ErrorInfo::invalidDataError(
    const std::string& message,
    const std::string& dataContext,
    const std::string& details) {
    
    ErrorInfo error(Category::InvalidData, Severity::Error,
                   generateErrorCode("INVDATA"), message);
    error.details = details;
    error.operation = "data_validation";
    
    if (!dataContext.empty()) {
        error.suggestedActions.push_back("Check data format: " + dataContext);
        error.suggestedActions.push_back("Verify data integrity");
        error.suggestedActions.push_back("Validate data source");
    }
    
    error.technicalActions.push_back("Review data validation rules");
    error.technicalActions.push_back("Check data transformation logic");
    error.technicalActions.push_back("Validate data schema compliance");
    
    return error;
}

std::string ErrorInfo::toString() const {
    std::ostringstream oss;
    oss << "[" << severityToString(severity) << "] " 
        << categoryToString(category) << " (" << code << "): " << message;
    
    if (!details.empty()) {
        oss << "\n  Details: " << details;
    }
    
    if (operation.has_value()) {
        oss << "\n  Operation: " << *operation;
    }
    
    if (component.has_value()) {
        oss << "\n  Component: " << *component;
    }
    
    if (filePath.has_value()) {
        oss << "\n  File: " << filePath->string();
    }
    
    if (url.has_value()) {
        oss << "\n  URL: " << *url;
    }
    
    if (externalAuthorityRelated) {
        oss << "\n  External Authority Related: Yes";
        if (externalSource.has_value()) {
            oss << " (Source: " << *externalSource << ")";
        }
    }
    
    if (!suggestedActions.empty()) {
        oss << "\n  Suggested Actions:";
        for (const auto& action : suggestedActions) {
            oss << "\n    - " << action;
        }
    }
    
    return oss.str();
}

std::map<std::string, std::string> ErrorInfo::toMap() const {
    std::map<std::string, std::string> result;
    
    result["severity"] = severityToString(severity);
    result["category"] = categoryToString(category);
    result["code"] = code;
    result["message"] = message;
    result["details"] = details;
    
    if (operation.has_value()) result["operation"] = *operation;
    if (component.has_value()) result["component"] = *component;
    if (filePath.has_value()) result["filePath"] = filePath->string();
    if (url.has_value()) result["url"] = *url;
    if (externalSource.has_value()) result["externalSource"] = *externalSource;
    
    result["externalAuthorityRelated"] = externalAuthorityRelated ? "true" : "false";
    result["isRecoverable"] = isRecoverable ? "true" : "false";
    
    auto now = std::chrono::system_clock::to_time_t(timestamp);
    result["timestamp"] = std::to_string(now);
    
    return result;
}

std::string ErrorInfo::categoryToString(Category category) {
    switch (category) {
        case Category::NetworkError: return "NetworkError";
        case Category::ExternalAuthorityError: return "ExternalAuthorityError";
        case Category::ValidationError: return "ValidationError";
        case Category::ParseError: return "ParseError";
        case Category::FileSystemError: return "FileSystemError";
        case Category::CacheError: return "CacheError";
        case Category::ConfigurationError: return "ConfigurationError";
        case Category::DependencyError: return "DependencyError";
        case Category::MemoryError: return "MemoryError";
        case Category::TimeoutError: return "TimeoutError";
        case Category::ConcurrencyError: return "ConcurrencyError";
        case Category::InvalidOperation: return "InvalidOperation";
        case Category::InvalidData: return "InvalidData";
        case Category::UnknownError: return "UnknownError";
        default: return "UnknownError";
    }
}

std::string ErrorInfo::severityToString(Severity severity) {
    switch (severity) {
        case Severity::Info: return "Info";
        case Severity::Warning: return "Warning";
        case Severity::Error: return "Error";
        case Severity::Critical: return "Critical";
        case Severity::Fatal: return "Fatal";
        default: return "Unknown";
    }
}

} // namespace dawproject