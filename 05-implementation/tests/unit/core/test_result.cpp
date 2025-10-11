/**
 * @file test_result.cpp
 * @brief Unit Tests for Result<T> Pattern (TDD Implementation)
 * 
 * Tests for DES-I-CORE-001 Result pattern implementation
 * Following Red-Green-Refactor TDD methodology
 * 
 * Traceability: DES-I-CORE-001 -> TEST-CORE-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <dawproject/core/result.h>
#include <dawproject/core/error_info.h>
#include <string>
#include <vector>

using namespace dawproject;

TEST_CASE("Result<T> - Success Construction", "[result][core]") {
    SECTION("Create success result with int") {
        auto result = Result<int>::success(42);
        
        REQUIRE(result.isSuccess());
        REQUIRE_FALSE(result.isError());
        REQUIRE(result.value() == 42);
    }
    
    SECTION("Create success result with string") {
        std::string testValue = "test_value";
        auto result = Result<std::string>::success(testValue);
        
        REQUIRE(result.isSuccess());
        REQUIRE_FALSE(result.isError());
        REQUIRE(result.value() == testValue);
    }
    
    SECTION("Create success result with move semantics") {
        std::vector<int> testVector{1, 2, 3, 4, 5};
        auto result = Result<std::vector<int>>::success(std::move(testVector));
        
        REQUIRE(result.isSuccess());
        REQUIRE(result.value().size() == 5);
        REQUIRE(result.value()[0] == 1);
        REQUIRE(result.value()[4] == 5);
    }
}

TEST_CASE("Result<T> - Error Construction", "[result][core]") {
    SECTION("Create error result with ErrorInfo") {
        auto errorInfo = ErrorInfo::validationError("Test validation error");
        auto result = Result<int>::error(errorInfo);
        
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE(result.isError());
        REQUIRE(result.error().message == "Test validation error");
        REQUIRE(result.error().category == ErrorInfo::Category::ValidationError);
    }
    
    SECTION("Error result should not allow value access") {
        auto errorInfo = ErrorInfo::parseError("Test parse error");
        auto result = Result<std::string>::error(errorInfo);
        
        REQUIRE_THROWS_AS(result.value(), std::runtime_error);
    }
    
    SECTION("Success result should not allow error access") {
        auto result = Result<int>::success(42);
        
        REQUIRE_THROWS_AS(result.error(), std::runtime_error);
    }
}

TEST_CASE("Result<void> - Specialized Implementation", "[result][core]") {
    SECTION("Create success void result") {
        auto result = Result<void>::success();
        
        REQUIRE(result.isSuccess());
        REQUIRE_FALSE(result.isError());
    }
    
    SECTION("Create error void result") {
        auto errorInfo = ErrorInfo::networkError("Network connection failed");
        auto result = Result<void>::error(errorInfo);
        
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE(result.isError());
        REQUIRE(result.error().message == "Network connection failed");
        REQUIRE(result.error().category == ErrorInfo::Category::NetworkError);
    }
}

TEST_CASE("Result<T> - Monadic Operations", "[result][core]") {
    SECTION("map() operation on success") {
        auto result = Result<int>::success(21);
        auto mapped = result.map([](int x) { return x * 2; });
        
        REQUIRE(mapped.isSuccess());
        REQUIRE(mapped.value() == 42);
    }
    
    SECTION("map() operation on error") {
        auto errorInfo = ErrorInfo::invalidDataError("Invalid data");
        auto result = Result<int>::error(errorInfo);
        auto mapped = result.map([](int x) { return x * 2; });
        
        REQUIRE_FALSE(mapped.isSuccess());
        REQUIRE(mapped.isError());
        REQUIRE(mapped.error().message == "Invalid data");
    }
    
    SECTION("flatMap() operation on success") {
        auto result = Result<int>::success(10);
        auto flatMapped = result.flatMap([](int x) -> Result<std::string> {
            if (x > 5) {
                return Result<std::string>::success("large number");
            } else {
                return Result<std::string>::error(
                    ErrorInfo::invalidDataError("Number too small"));
            }
        });
        
        REQUIRE(flatMapped.isSuccess());
        REQUIRE(flatMapped.value() == "large number");
    }
    
    SECTION("flatMap() operation on error") {
        auto errorInfo = ErrorInfo::configurationError("Config invalid", "test");
        auto result = Result<int>::error(errorInfo);
        auto flatMapped = result.flatMap([](int x) -> Result<std::string> {
            (void)x; // Suppress unused parameter warning
            return Result<std::string>::success("should not be called");
        });
        
        REQUIRE_FALSE(flatMapped.isSuccess());
        REQUIRE(flatMapped.isError());
        REQUIRE(flatMapped.error().message == "Config invalid");
    }
    
    SECTION("mapError() operation") {
        auto errorInfo = ErrorInfo::fileSystemError("File not found");
        auto result = Result<int>::error(errorInfo);
        
        auto mappedError = result.mapError([](const ErrorInfo& err) -> ErrorInfo {
            auto newError = ErrorInfo::fileSystemError("Enhanced: " + err.message);
            newError.suggestedActions.push_back("Check file path");
            return newError;
        });
        
        REQUIRE_FALSE(mappedError.isSuccess());
        REQUIRE(mappedError.error().message == "Enhanced: File not found");
        REQUIRE(mappedError.error().suggestedActions.size() == 1);
        REQUIRE(mappedError.error().suggestedActions[0] == "Check file path");
    }
}

TEST_CASE("Result<T> - Utility Operations", "[result][core]") {
    SECTION("valueOr() with success") {
        auto result = Result<int>::success(42);
        int defaultValue = 99;
        
        REQUIRE(result.valueOr(defaultValue) == 42);
    }
    
    SECTION("valueOr() with error") {
        auto errorInfo = ErrorInfo::timeoutError("Operation timeout", std::chrono::milliseconds(1000));
        auto result = Result<int>::error(errorInfo);
        int defaultValue = 99;
        
        REQUIRE(result.valueOr(defaultValue) == 99);
    }
    
    SECTION("valueOrElse() with success") {
        auto result = Result<std::string>::success("actual_value");
        
        auto value = result.valueOrElse([]() { return std::string("fallback"); });
        REQUIRE(value == "actual_value");
    }
    
    SECTION("valueOrElse() with error") {
        auto errorInfo = ErrorInfo::memoryError("Out of memory");
        auto result = Result<std::string>::error(errorInfo);
        
        auto value = result.valueOrElse([]() { return std::string("fallback"); });
        REQUIRE(value == "fallback");
    }
}

TEST_CASE("Result<T> - Copy and Move Semantics", "[result][core]") {
    SECTION("Copy success result") {
        auto original = Result<int>::success(42);
        auto copy = original;
        
        REQUIRE(copy.isSuccess());
        REQUIRE(copy.value() == 42);
        REQUIRE(original.isSuccess());
        REQUIRE(original.value() == 42);
    }
    
    SECTION("Move success result") {
        auto original = Result<std::vector<int>>::success(std::vector<int>{1, 2, 3});
        auto moved = std::move(original);
        
        REQUIRE(moved.isSuccess());
        REQUIRE(moved.value().size() == 3);
        REQUIRE(moved.value()[0] == 1);
    }
    
    SECTION("Copy error result") {
        auto errorInfo = ErrorInfo::dependencyError("Missing lib");
        auto original = Result<int>::error(errorInfo);
        auto copy = original;
        
        REQUIRE(copy.isError());
        REQUIRE(copy.error().message == "Missing lib");
        REQUIRE(original.isError());
    }
}

TEST_CASE("Result<void> - Monadic Operations", "[result][core]") {
    SECTION("map() void to value") {
        auto result = Result<void>::success();
        auto mapped = result.map([]() { return 42; });
        
        REQUIRE(mapped.isSuccess());
        REQUIRE(mapped.value() == 42);
    }
    
    SECTION("flatMap() void to Result") {
        auto result = Result<void>::success();
        auto flatMapped = result.flatMap([]() -> Result<std::string> {
            return Result<std::string>::success("success");
        });
        
        REQUIRE(flatMapped.isSuccess());
        REQUIRE(flatMapped.value() == "success");
    }
    
    SECTION("map() error propagation") {
        auto errorInfo = ErrorInfo::externalAuthorityError("External auth failed", "test.com");
        auto result = Result<void>::error(errorInfo);
        auto mapped = result.map([]() { return 42; });
        
        REQUIRE_FALSE(mapped.isSuccess());
        REQUIRE(mapped.error().message == "External auth failed");
    }
}

// External Authority Integration Tests
TEST_CASE("Result<T> - External Authority Error Context", "[result][core][external-authority]") {
    SECTION("External authority error carries compliance context") {
        auto errorInfo = ErrorInfo::externalAuthorityError(
            "Schema validation failed", 
            "dawproject.org/schema/v1.0");
        errorInfo.schemaVersion = "1.0";
        errorInfo.externalAuthorityRelated = true;
        
        auto result = Result<int>::error(errorInfo);
        
        REQUIRE(result.isError());
        REQUIRE(result.error().externalAuthorityRelated);
        REQUIRE(result.error().externalSource.value() == "dawproject.org/schema/v1.0");
        REQUIRE(result.error().schemaVersion.value() == "1.0");
        REQUIRE(result.error().category == ErrorInfo::Category::ExternalAuthorityError);
    }
    
    SECTION("Error chaining with external authority context") {
        auto rootError = ErrorInfo::networkError("Connection timeout");
        auto authError = ErrorInfo::externalAuthorityError(
            "Failed to download external schema", 
            "github.com/dawproject");
        authError.cause = std::make_shared<ErrorInfo>(rootError);
        
        auto result = Result<void>::error(authError);
        
        REQUIRE(result.isError());
        REQUIRE(result.error().cause != nullptr);
        REQUIRE(result.error().cause->category == ErrorInfo::Category::NetworkError);
        REQUIRE(result.error().externalAuthorityRelated);
    }
}

// Performance Tests
TEST_CASE("Result<T> - Performance Characteristics", "[result][core][performance]") {
    SECTION("Zero-overhead for success case") {
        // This test verifies that Result<T> doesn't add unnecessary overhead
        auto result = Result<int>::success(42);
        
        // Should be efficient to check success and access value
        for (int i = 0; i < 1000; ++i) {
            if (result.isSuccess()) {
                auto value = result.value();
                REQUIRE(value == 42);
            }
        }
    }
    
    SECTION("Efficient error handling") {
        auto errorInfo = ErrorInfo::validationError("Test error");
        auto result = Result<int>::error(errorInfo);
        
        // Should be efficient to check error and access error info
        for (int i = 0; i < 1000; ++i) {
            if (result.isError()) {
                auto& error = result.error();
                REQUIRE(error.category == ErrorInfo::Category::ValidationError);
            }
        }
    }
}