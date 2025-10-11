/**
 * @file test_error_info.cpp
 * @brief Unit Tests for ErrorInfo Implementation (TDD Implementation)
 * 
 * Tests for DES-I-CORE-001 ErrorInfo implementation
 * Following Red-Green-Refactor TDD methodology
 * 
 * Traceability: DES-I-CORE-001 -> TEST-CORE-002
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <catch2/catch_test_macros.hpp>
#include <dawproject/core/error_info.h>
#include <filesystem>
#include <chrono>

using namespace dawproject;

TEST_CASE("ErrorInfo - Basic Construction", "[error-info][core]") {
    SECTION("Default constructor") {
        ErrorInfo error;
        
        REQUIRE(error.category == ErrorInfo::Category::UnknownError);
        REQUIRE(error.severity == ErrorInfo::Severity::Error);
        REQUIRE(error.code.empty());
        REQUIRE(error.message.empty());
        REQUIRE(error.isRecoverable == true);
        REQUIRE(error.externalAuthorityRelated == false);
    }
    
    SECTION("Constructor with parameters") {
        ErrorInfo error(ErrorInfo::Category::ValidationError, 
                       ErrorInfo::Severity::Critical,
                       "VAL_001", 
                       "Validation failed");
        
        REQUIRE(error.category == ErrorInfo::Category::ValidationError);
        REQUIRE(error.severity == ErrorInfo::Severity::Critical);
        REQUIRE(error.code == "VAL_001");
        REQUIRE(error.message == "Validation failed");
    }
}

TEST_CASE("ErrorInfo - Network Error Factory", "[error-info][core][factory]") {
    SECTION("Basic network error") {
        auto error = ErrorInfo::networkError("Connection failed");
        
        REQUIRE(error.category == ErrorInfo::Category::NetworkError);
        REQUIRE(error.severity == ErrorInfo::Severity::Error);
        REQUIRE(error.message == "Connection failed");
        REQUIRE(error.code.substr(0, 3) == "NET");
        REQUIRE(error.operation.value() == "network_operation");
        REQUIRE(error.suggestedActions.size() >= 1);
        REQUIRE(error.technicalActions.size() >= 1);
    }
    
    SECTION("Network error with URL") {
        auto error = ErrorInfo::networkError("Timeout", "https://example.com", "Connection timeout after 30s");
        
        REQUIRE(error.url.value() == "https://example.com");
        REQUIRE(error.details == "Connection timeout after 30s");
        
        // Should have URL-specific suggested actions
        bool hasUrlAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("https://example.com") != std::string::npos) {
                hasUrlAction = true;
                break;
            }
        }
        REQUIRE(hasUrlAction);
    }
}

TEST_CASE("ErrorInfo - External Authority Error Factory", "[error-info][core][factory][external-authority]") {
    SECTION("Basic external authority error") {
        auto error = ErrorInfo::externalAuthorityError("Schema not found", "dawproject.org");
        
        REQUIRE(error.category == ErrorInfo::Category::ExternalAuthorityError);
        REQUIRE(error.severity == ErrorInfo::Severity::Critical);
        REQUIRE(error.message == "Schema not found");
        REQUIRE(error.externalSource.value() == "dawproject.org");
        REQUIRE(error.externalAuthorityRelated == true);
        REQUIRE(error.operation.value() == "external_authority_compliance");
        REQUIRE(error.code.substr(0, 3) == "EXT");
    }
    
    SECTION("External authority error with details") {
        auto error = ErrorInfo::externalAuthorityError(
            "Compliance check failed", 
            "github.com/bitwig/dawproject",
            "XSD validation returned 3 errors");
        
        REQUIRE(error.details == "XSD validation returned 3 errors");
        REQUIRE(error.externalAuthorityRelated == true);
        
        // Should have external authority specific actions
        bool hasAuthorityAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("external authority") != std::string::npos) {
                hasAuthorityAction = true;
                break;
            }
        }
        REQUIRE(hasAuthorityAction);
    }
}

TEST_CASE("ErrorInfo - Validation Error Factory", "[error-info][core][factory]") {
    SECTION("Validation error with file path") {
        std::filesystem::path testPath = "/path/to/test.dawproject";
        auto error = ErrorInfo::validationError("Invalid format", testPath, "Missing required elements");
        
        REQUIRE(error.category == ErrorInfo::Category::ValidationError);
        REQUIRE(error.severity == ErrorInfo::Severity::Error);
        REQUIRE(error.message == "Invalid format");
        REQUIRE(error.filePath.value() == testPath);
        REQUIRE(error.details == "Missing required elements");
        REQUIRE(error.operation.value() == "validation");
        REQUIRE(error.code.substr(0, 3) == "VAL");
    }
    
    SECTION("Validation error without file path") {
        auto error = ErrorInfo::validationError("Generic validation error");
        
        REQUIRE(error.category == ErrorInfo::Category::ValidationError);
        REQUIRE_FALSE(error.filePath.has_value());
        REQUIRE(error.suggestedActions.size() >= 1);
    }
}

TEST_CASE("ErrorInfo - Parse Error Factory", "[error-info][core][factory]") {
    SECTION("Parse error with file path") {
        std::filesystem::path xmlPath = "/data/project.xml";
        auto error = ErrorInfo::parseError("XML malformed", xmlPath, "Unclosed tag at line 42");
        
        REQUIRE(error.category == ErrorInfo::Category::ParseError);
        REQUIRE(error.message == "XML malformed");
        REQUIRE(error.filePath.value() == xmlPath);
        REQUIRE(error.details == "Unclosed tag at line 42");
        REQUIRE(error.code.substr(0, 5) == "PARSE");
    }
}

TEST_CASE("ErrorInfo - FileSystem Error Factory", "[error-info][core][factory]") {
    SECTION("File system error") {
        std::filesystem::path filePath = "/missing/file.dawproject";
        auto error = ErrorInfo::fileSystemError("File not found", filePath, "Permission denied");
        
        REQUIRE(error.category == ErrorInfo::Category::FileSystemError);
        REQUIRE(error.message == "File not found");
        REQUIRE(error.filePath.value() == filePath);
        REQUIRE(error.details == "Permission denied");
        REQUIRE(error.code.substr(0, 2) == "FS");
        
        // Should suggest file-related actions
        bool hasFileAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("file") != std::string::npos || action.find("File") != std::string::npos) {
                hasFileAction = true;
                break;
            }
        }
        REQUIRE(hasFileAction);
    }
}

TEST_CASE("ErrorInfo - Configuration Error Factory", "[error-info][core][factory]") {
    SECTION("Configuration error with component") {
        std::vector<std::string> actions = {"Check config.json", "Restore defaults"};
        auto error = ErrorInfo::configurationError("Invalid settings", "XMLProcessor", actions);
        
        REQUIRE(error.category == ErrorInfo::Category::ConfigurationError);
        REQUIRE(error.message == "Invalid settings");
        REQUIRE(error.component.value() == "XMLProcessor");
        REQUIRE(error.suggestedActions == actions);
        REQUIRE(error.code.substr(0, 3) == "CFG");
    }
    
    SECTION("Configuration error without actions") {
        auto error = ErrorInfo::configurationError("Bad config", "TestComponent");
        
        REQUIRE(error.suggestedActions.size() >= 3); // Should have default actions
        REQUIRE(error.technicalActions.size() >= 1);
    }
}

TEST_CASE("ErrorInfo - Dependency Error Factory", "[error-info][core][factory]") {
    SECTION("Dependency error with install instructions") {
        std::vector<std::string> instructions = {"sudo apt install libxml2-dev", "brew install libxml2"};
        auto error = ErrorInfo::dependencyError("libxml2", "Version 2.9+ required", instructions);
        
        REQUIRE(error.category == ErrorInfo::Category::DependencyError);
        REQUIRE(error.severity == ErrorInfo::Severity::Critical);
        REQUIRE(error.message.find("libxml2") != std::string::npos);
        REQUIRE(error.details == "Version 2.9+ required");
        
        // Should include custom install instructions
        bool hasCustomInstruction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("sudo apt install") != std::string::npos) {
                hasCustomInstruction = true;
                break;
            }
        }
        REQUIRE(hasCustomInstruction);
    }
}

TEST_CASE("ErrorInfo - Timeout Error Factory", "[error-info][core][factory]") {
    SECTION("Timeout error with duration") {
        auto timeout = std::chrono::milliseconds(5000);
        auto error = ErrorInfo::timeoutError("external_validation", timeout, "XSD validation timeout");
        
        REQUIRE(error.category == ErrorInfo::Category::TimeoutError);
        REQUIRE(error.operation.value() == "external_validation");
        REQUIRE(error.duration.value() == timeout);
        REQUIRE(error.details == "XSD validation timeout");
        
        // Should suggest timeout-related actions
        bool hasTimeoutAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("timeout") != std::string::npos && action.find("5000ms") != std::string::npos) {
                hasTimeoutAction = true;
                break;
            }
        }
        REQUIRE(hasTimeoutAction);
    }
}

TEST_CASE("ErrorInfo - Memory Error Factory", "[error-info][core][factory]") {
    SECTION("Memory error with requested size") {
        size_t requestedSize = 100 * 1024 * 1024; // 100MB
        auto error = ErrorInfo::memoryError("Allocation failed", requestedSize, "Out of memory");
        
        REQUIRE(error.category == ErrorInfo::Category::MemoryError);
        REQUIRE(error.severity == ErrorInfo::Severity::Critical);
        REQUIRE(error.memoryUsage.value() == requestedSize);
        REQUIRE(error.details == "Out of memory");
        
        // Should suggest memory-related actions
        bool hasMemoryAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("memory") != std::string::npos && action.find("100MB") != std::string::npos) {
                hasMemoryAction = true;
                break;
            }
        }
        REQUIRE(hasMemoryAction);
    }
}

TEST_CASE("ErrorInfo - Invalid Operation Error Factory", "[error-info][core][factory]") {
    SECTION("Invalid operation error") {
        auto error = ErrorInfo::invalidOperationError(
            "write_project", 
            "External validation not configured", 
            "Must configure external authority before writing");
        
        REQUIRE(error.category == ErrorInfo::Category::InvalidOperation);
        REQUIRE(error.operation.value() == "write_project");
        REQUIRE(error.details.find("External validation not configured") != std::string::npos);
        REQUIRE(error.details.find("Must configure external authority") != std::string::npos);
    }
}

TEST_CASE("ErrorInfo - Invalid Data Error Factory", "[error-info][core][factory]") {
    SECTION("Invalid data error with context") {
        auto error = ErrorInfo::invalidDataError(
            "Malformed XML data", 
            "project.xml", 
            "Invalid character encoding");
        
        REQUIRE(error.category == ErrorInfo::Category::InvalidData);
        REQUIRE(error.message == "Malformed XML data");
        REQUIRE(error.details == "Invalid character encoding");
        
        // Should suggest data-related actions
        bool hasDataAction = false;
        for (const auto& action : error.suggestedActions) {
            if (action.find("project.xml") != std::string::npos) {
                hasDataAction = true;
                break;
            }
        }
        REQUIRE(hasDataAction);
    }
}

TEST_CASE("ErrorInfo - String Conversion", "[error-info][core][utility]") {
    SECTION("toString() comprehensive format") {
        auto error = ErrorInfo::externalAuthorityError(
            "Schema validation failed",
            "dawproject.org");
        error.details = "Missing required attributes";
        error.operation = "external_validation";
        error.component = "ValidationEngine";
        error.filePath = std::filesystem::path("/test/file.dawproject");
        error.url = "https://dawproject.org/schema/v1.0.xsd";
        error.suggestedActions = {"Update schema", "Check file format"};
        
        std::string result = error.toString();
        
        REQUIRE(result.find("Schema validation failed") != std::string::npos);
        REQUIRE(result.find("Missing required attributes") != std::string::npos);
        REQUIRE(result.find("external_validation") != std::string::npos);
        REQUIRE(result.find("ValidationEngine") != std::string::npos);
        REQUIRE(result.find("/test/file.dawproject") != std::string::npos);
        REQUIRE(result.find("https://dawproject.org") != std::string::npos);
        REQUIRE(result.find("Update schema") != std::string::npos);
        REQUIRE(result.find("External Authority Related: Yes") != std::string::npos);
    }
}

TEST_CASE("ErrorInfo - Map Conversion", "[error-info][core][utility]") {
    SECTION("toMap() with all fields") {
        auto error = ErrorInfo::networkError("Connection timeout", "https://api.example.com");
        error.component = "HTTPClient";
        
        auto map = error.toMap();
        
        REQUIRE(map["severity"] == "Error");
        REQUIRE(map["category"] == "NetworkError");
        REQUIRE(map["message"] == "Connection timeout");
        REQUIRE(map["url"] == "https://api.example.com");
        REQUIRE(map["component"] == "HTTPClient");
        REQUIRE(map["externalAuthorityRelated"] == "false");
        REQUIRE(map["isRecoverable"] == "true");
        REQUIRE(map.find("timestamp") != map.end());
    }
}

TEST_CASE("ErrorInfo - Category and Severity String Conversion", "[error-info][core][utility]") {
    SECTION("Category to string conversion") {
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::NetworkError) == "NetworkError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::ExternalAuthorityError) == "ExternalAuthorityError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::ValidationError) == "ValidationError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::ParseError) == "ParseError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::FileSystemError) == "FileSystemError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::ConfigurationError) == "ConfigurationError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::DependencyError) == "DependencyError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::MemoryError) == "MemoryError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::TimeoutError) == "TimeoutError");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::InvalidOperation) == "InvalidOperation");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::InvalidData) == "InvalidData");
        REQUIRE(ErrorInfo::categoryToString(ErrorInfo::Category::UnknownError) == "UnknownError");
    }
    
    SECTION("Severity to string conversion") {
        REQUIRE(ErrorInfo::severityToString(ErrorInfo::Severity::Info) == "Info");
        REQUIRE(ErrorInfo::severityToString(ErrorInfo::Severity::Warning) == "Warning");
        REQUIRE(ErrorInfo::severityToString(ErrorInfo::Severity::Error) == "Error");
        REQUIRE(ErrorInfo::severityToString(ErrorInfo::Severity::Critical) == "Critical");
        REQUIRE(ErrorInfo::severityToString(ErrorInfo::Severity::Fatal) == "Fatal");
    }
}

// External Authority Integration Tests
TEST_CASE("ErrorInfo - External Authority Context", "[error-info][core][external-authority]") {
    SECTION("External authority error carries full context") {
        auto error = ErrorInfo::externalAuthorityError(
            "XSD validation failed", 
            "github.com/bitwig/dawproject");
        error.schemaVersion = "1.0.0";
        error.externalSource = "github.com/bitwig/dawproject";
        error.externalAuthorityRelated = true;
        
        REQUIRE(error.externalAuthorityRelated);
        REQUIRE(error.externalSource.value() == "github.com/bitwig/dawproject");
        REQUIRE(error.schemaVersion.value() == "1.0.0");
        REQUIRE(error.category == ErrorInfo::Category::ExternalAuthorityError);
        
        auto str = error.toString();
        REQUIRE(str.find("External Authority Related: Yes") != std::string::npos);
        REQUIRE(str.find("github.com/bitwig/dawproject") != std::string::npos);
    }
    
    SECTION("Error chaining for external authority failures") {
        auto networkError = ErrorInfo::networkError("DNS resolution failed");
        auto authError = ErrorInfo::externalAuthorityError(
            "Cannot download schema", 
            "dawproject.org");
        authError.cause = std::make_shared<ErrorInfo>(networkError);
        
        REQUIRE(authError.cause != nullptr);
        REQUIRE(authError.cause->category == ErrorInfo::Category::NetworkError);
        REQUIRE(authError.externalAuthorityRelated);
    }
}

TEST_CASE("ErrorInfo - Error Code Generation", "[error-info][core][utility]") {
    SECTION("Error codes are unique and properly formatted") {
        auto error1 = ErrorInfo::networkError("First error");
        auto error2 = ErrorInfo::networkError("Second error");
        auto error3 = ErrorInfo::validationError("Validation error");
        
        // Codes should be unique
        REQUIRE(error1.code != error2.code);
        REQUIRE(error2.code != error3.code);
        REQUIRE(error1.code != error3.code);
        
        // Codes should have proper prefixes
        REQUIRE(error1.code.substr(0, 3) == "NET");
        REQUIRE(error2.code.substr(0, 3) == "NET");
        REQUIRE(error3.code.substr(0, 3) == "VAL");
        
        // Codes should have numeric suffixes
        REQUIRE(error1.code.length() > 4);
        REQUIRE(error2.code.length() > 4);
        REQUIRE(error3.code.length() > 4);
    }
}