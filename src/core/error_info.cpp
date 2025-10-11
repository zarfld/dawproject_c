//==============================================================================
// DAWProject C++ Library - Phase 05 Implementation
// ErrorInfo Factory Methods Implementation - TDD Green Phase
// 
// Standards Compliance:
//   - XP Practices: Minimal implementation to make tests pass (GREEN)
//   - ISO/IEC/IEEE 12207:2017 Implementation Process
//   - Design Traceability: DES-I-CORE-001 -> IMP-CORE-001
//
// TDD Status: GREEN Phase - Making RED tests pass with minimal implementation
//==============================================================================

#include "dawproject/core/result.hpp"
#include <sstream>
#include <algorithm>
#include <chrono>

namespace dawproject {

//==============================================================================
// ErrorInfo Factory Methods - Network Errors
//==============================================================================

ErrorInfo ErrorInfo::networkError(
    const std::string& message,
    const std::string& url,
    const std::string& details) {
    
    ErrorInfo error;
    error.category = Category::NetworkError;
    error.severity = Severity::Error;
    error.message = message;
    error.details = details;
    error.url = url;
    error.externalAuthorityRelated = true;  // Network errors often relate to external resources
    error.isRecoverable = true;  // Network issues can often be retried
    error.timestamp = std::chrono::system_clock::now();
    
    // Generate unique error code
    error.code = generateErrorCode("NET", error.category);
    
    // Default suggested actions for network errors
    error.suggestedActions = {
        "Check network connectivity",
        "Verify URL accessibility: " + url,
        "Retry operation after brief delay",
        "Check firewall and proxy settings"
    };
    
    return error;
}

//==============================================================================
// ErrorInfo Factory Methods - External Authority Errors
//==============================================================================

ErrorInfo ErrorInfo::externalAuthorityError(
    const std::string& message,
    const std::string& externalSource,
    const std::string& details) {
    
    ErrorInfo error;
    error.category = Category::ExternalAuthorityError;
    error.severity = Severity::Critical;  // External authority errors are critical
    error.message = message;
    error.details = details;
    error.externalSource = externalSource;
    error.externalAuthorityRelated = true;  // By definition
    error.isRecoverable = false;  // Requires external authority fix
    error.timestamp = std::chrono::system_clock::now();
    
    // Generate unique error code
    error.code = generateErrorCode("EXT", error.category);
    
    // Default suggested actions for external authority errors
    error.suggestedActions = {
        "Verify external authority specification compliance",
        "Check " + externalSource + " for updates or changes",
        "Review DAWProject specification version compatibility",
        "Contact external authority maintainers if issue persists"
    };
    
    return error;
}

//==============================================================================
// ErrorInfo Factory Methods - Validation Errors  
//==============================================================================

ErrorInfo ErrorInfo::validationError(
    const std::string& message,
    const std::filesystem::path& filePath,
    const std::string& details) {
    
    ErrorInfo error;
    error.category = Category::ValidationError;
    error.severity = Severity::Error;
    error.message = message;
    error.details = details;
    if (!filePath.empty()) {
        error.filePath = filePath;
    }
    error.externalAuthorityRelated = false;  // Internal validation unless specified
    error.isRecoverable = true;  // Can often be fixed by correcting input
    error.timestamp = std::chrono::system_clock::now();
    
    // Generate unique error code
    error.code = generateErrorCode("VAL", error.category);
    
    // Default suggested actions for validation errors
    error.suggestedActions = {
        "Review input data format and structure",
        "Verify against DAWProject specification",
        "Check for required elements and attributes"
    };
    
    if (!filePath.empty()) {
        error.suggestedActions.push_back("Examine file: " + filePath.string());
    }
    
    return error;
}

//==============================================================================
// ErrorInfo Factory Methods - Overloaded Validation Error (No File Path)
//==============================================================================

ErrorInfo ErrorInfo::validationError(const std::string& message) {
    return validationError(message, std::filesystem::path{}, "");
}

//==============================================================================
// ErrorInfo Factory Methods - Configuration Errors
//==============================================================================

ErrorInfo ErrorInfo::configurationError(
    const std::string& message,
    const std::string& component,
    const std::string& details) {
    
    ErrorInfo error;
    error.category = Category::ConfigurationError;
    error.severity = Severity::Warning;  // Config errors often non-fatal
    error.message = message;
    error.details = details;
    error.component = component;
    error.externalAuthorityRelated = false;  // Internal configuration
    error.isRecoverable = true;  // Config can usually be corrected
    error.timestamp = std::chrono::system_clock::now();
    
    // Generate unique error code
    error.code = generateErrorCode("CFG", error.category);
    
    // Default suggested actions for configuration errors
    error.suggestedActions = {
        "Review component configuration: " + component,
        "Verify configuration file syntax and values",
        "Check for missing or invalid configuration parameters",
        "Restore to default configuration if necessary"
    };
    
    return error;
}

//==============================================================================
// ErrorInfo Factory Methods - Dependency Errors
//==============================================================================

ErrorInfo ErrorInfo::dependencyError(
    const std::string& dependency,
    const std::string& details,
    const std::vector<std::string>& installInstructions) {
    
    ErrorInfo error;
    error.category = Category::DependencyError;
    error.severity = Severity::Fatal;  // Missing dependencies are fatal
    error.message = "Missing or incompatible dependency: " + dependency;
    error.details = details;
    error.externalAuthorityRelated = false;  // Dependencies are internal concerns
    error.isRecoverable = false;  // Requires manual installation
    error.timestamp = std::chrono::system_clock::now();
    
    // Generate unique error code
    error.code = generateErrorCode("DEP", error.category);
    
    // Use provided install instructions or defaults
    if (!installInstructions.empty()) {
        error.suggestedActions = installInstructions;
    } else {
        error.suggestedActions = {
            "Install required dependency: " + dependency,
            "Update system package manager",
            "Verify installation and version compatibility"
        };
    }
    
    return error;
}

//==============================================================================
// ErrorInfo Utility Methods - Error Code Generation
//==============================================================================

std::string ErrorInfo::generateErrorCode(
    const std::string& prefix, 
    Category category) {
    
    // Create timestamp-based unique suffix
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream code_stream;
    code_stream << prefix << "-" 
                << static_cast<int>(category) << "-"
                << time_t << "-"
                << ms.count();
    
    return code_stream.str();
}

//==============================================================================
// ErrorInfo Utility Methods - Detection Methods
//==============================================================================

bool ErrorInfo::isExternalAuthorityError() const {
    return category == Category::ExternalAuthorityError || 
           externalAuthorityRelated;
}

//==============================================================================
// ErrorInfo Utility Methods - Message Formatting
//==============================================================================

std::string ErrorInfo::getFormattedMessage(bool includeDetails) const {
    std::ostringstream formatted;
    
    // Basic message
    formatted << message;
    
    if (includeDetails) {
        // Add component context if available
        if (!component.empty()) {
            formatted << " [Component: " << component << "]";
        }
        
        // Add operation context if available
        if (!operation.empty()) {
            formatted << " [Operation: " << operation << "]";
        }
        
        // Add details if available
        if (!details.empty()) {
            formatted << " - " << details;
        }
        
        // Add file path if available
        if (filePath.has_value()) {
            formatted << " (File: " << filePath->string() << ")";
        }
        
        // Add URL if available
        if (url.has_value()) {
            formatted << " (URL: " << url.value() << ")";
        }
        
        // Add external source if available
        if (externalSource.has_value()) {
            formatted << " (External Authority: " << externalSource.value() << ")";
        }
        
        // Add error code
        if (!code.empty()) {
            formatted << " [Code: " << code << "]";
        }
    }
    
    return formatted.str();
}

//==============================================================================
// ErrorInfo Utility Methods - Error Chain Navigation
//==============================================================================

std::vector<ErrorInfo> ErrorInfo::getAllErrors() const {
    std::vector<ErrorInfo> all_errors;
    
    // Add current error
    all_errors.push_back(*this);
    
    // Add related errors
    for (const auto& related : relatedErrors) {
        all_errors.push_back(related);
        
        // Recursively add errors from related errors
        auto related_chain = related.getAllErrors();
        all_errors.insert(all_errors.end(), 
                         related_chain.begin() + 1,  // Skip duplicate of related error itself
                         related_chain.end());
    }
    
    // Add cause chain
    if (cause) {
        auto cause_chain = cause->getAllErrors();
        all_errors.insert(all_errors.end(), 
                         cause_chain.begin(), 
                         cause_chain.end());
    }
    
    // Remove duplicates (by error code)
    std::sort(all_errors.begin(), all_errors.end(), 
              [](const ErrorInfo& a, const ErrorInfo& b) {
                  return a.code < b.code;
              });
    
    auto unique_end = std::unique(all_errors.begin(), all_errors.end(),
                                  [](const ErrorInfo& a, const ErrorInfo& b) {
                                      return a.code == b.code;
                                  });
    
    all_errors.erase(unique_end, all_errors.end());
    
    return all_errors;
}

} // namespace dawproject