//==============================================================================
// DAWProject C++ Library - Phase 05 Implementation
// Result<T> Pattern Tests - TDD Red-Green-Refactor Cycle  
// 
// Standards Compliance:
//   - XP Practices: Test-Driven Development (TDD)
//   - ISO/IEC/IEEE 12207:2017 Implementation Process
//   - Design Traceability: DES-I-CORE-001 -> TEST-CORE-001
//
// TDD Cycle: RED (Failing Tests) -> GREEN (Minimal Implementation) -> REFACTOR
//==============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "dawproject/core/result.hpp"

using namespace dawproject;
using namespace testing;

//==============================================================================
// Test Fixture for Result<T> Pattern (TDD Structure)
//==============================================================================

class ResultTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for each test case
        test_string_value_ = "test_success_value";
        test_int_value_ = 42;
        
        // Sample error info for testing
        sample_error_ = ErrorInfo::validationError(
            "Test validation error",
            std::filesystem::path("/test/path/file.dawproject"),
            "Detailed test error information");
    }
    
    void TearDown() override {
        // Cleanup after each test case
    }
    
    // Test data
    std::string test_string_value_;
    int test_int_value_;
    ErrorInfo sample_error_;
};

//==============================================================================
// RED Phase: ErrorInfo Factory Methods Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, ErrorInfo_NetworkError_CreatesCorrectErrorInfo) {
    // GIVEN: Network error parameters
    const std::string message = "Connection timeout";
    const std::string url = "https://raw.githubusercontent.com/bitwig/dawproject/main/Project.xsd";
    const std::string details = "External authority schema download failed";
    
    // WHEN: Creating network error
    auto error = ErrorInfo::networkError(message, url, details);
    
    // THEN: Error should have correct properties
    EXPECT_EQ(error.category, ErrorInfo::Category::NetworkError);
    EXPECT_EQ(error.severity, ErrorInfo::Severity::Error);
    EXPECT_EQ(error.message, message);
    EXPECT_EQ(error.details, details);
    EXPECT_EQ(error.url.value(), url);
    EXPECT_TRUE(error.externalAuthorityRelated);
    EXPECT_FALSE(error.code.empty());
    EXPECT_FALSE(error.suggestedActions.empty());
}

TEST_F(ResultTest, ErrorInfo_ExternalAuthorityError_CreatesCorrectErrorInfo) {
    // GIVEN: External authority error parameters
    const std::string message = "XSD validation failed";
    const std::string externalSource = "DAWProject Official Schema";
    const std::string details = "Element 'Track' validation failed";
    
    // WHEN: Creating external authority error
    auto error = ErrorInfo::externalAuthorityError(message, externalSource, details);
    
    // THEN: Error should indicate external authority issue
    EXPECT_EQ(error.category, ErrorInfo::Category::ExternalAuthorityError);
    EXPECT_EQ(error.severity, ErrorInfo::Severity::Critical);
    EXPECT_EQ(error.message, message);
    EXPECT_EQ(error.details, details);
    EXPECT_EQ(error.externalSource.value(), externalSource);
    EXPECT_TRUE(error.externalAuthorityRelated);
    EXPECT_TRUE(error.isExternalAuthorityError());
}

TEST_F(ResultTest, ErrorInfo_ValidationError_CreatesCorrectErrorInfo) {
    // GIVEN: Validation error parameters
    const std::string message = "Invalid XML structure";
    const std::filesystem::path filePath = "/path/to/project.dawproject";
    const std::string details = "Missing required element 'Timeline'";
    
    // WHEN: Creating validation error
    auto error = ErrorInfo::validationError(message, filePath, details);
    
    // THEN: Error should have validation context
    EXPECT_EQ(error.category, ErrorInfo::Category::ValidationError);
    EXPECT_EQ(error.severity, ErrorInfo::Severity::Error);
    EXPECT_EQ(error.message, message);
    EXPECT_EQ(error.details, details);
    EXPECT_EQ(error.filePath.value(), filePath);
    EXPECT_FALSE(error.externalAuthorityRelated);  // Not external by default
}

TEST_F(ResultTest, ErrorInfo_DependencyError_CreatesCorrectErrorInfo) {
    // GIVEN: Dependency error parameters
    const std::string dependency = "libxml2";
    const std::string details = "Version 2.9.10+ required for XSD validation";
    const std::vector<std::string> installInstructions = {
        "Ubuntu: sudo apt-get install libxml2-dev",
        "macOS: brew install libxml2",
        "Windows: vcpkg install libxml2"
    };
    
    // WHEN: Creating dependency error
    auto error = ErrorInfo::dependencyError(dependency, details, installInstructions);
    
    // THEN: Error should have installation instructions
    EXPECT_EQ(error.category, ErrorInfo::Category::DependencyError);
    EXPECT_EQ(error.severity, ErrorInfo::Severity::Fatal);
    EXPECT_THAT(error.message, HasSubstr(dependency));
    EXPECT_EQ(error.details, details);
    EXPECT_EQ(error.suggestedActions, installInstructions);
    EXPECT_FALSE(error.isRecoverable);  // Dependency errors typically not auto-recoverable
}

//==============================================================================
// RED Phase: Result<T> Success Cases Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, Result_Success_CreatesSuccessResult) {
    // WHEN: Creating success result
    auto result = Result<std::string>::success(test_string_value_);
    
    // THEN: Result should represent success
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_FALSE(result.isExternalAuthorityError());
    EXPECT_EQ(result.value(), test_string_value_);
}

TEST_F(ResultTest, Result_Success_ValueAccessMethods) {
    // GIVEN: Success result
    auto result = Result<int>::success(test_int_value_);
    
    // THEN: All value access methods should work
    EXPECT_EQ(result.value(), test_int_value_);
    EXPECT_EQ(static_cast<const Result<int>&>(result).value(), test_int_value_);
    
    // Test mutable access
    result.value() = 99;
    EXPECT_EQ(result.value(), 99);
    
    // Test move access
    auto moved_result = Result<int>::success(123);
    int moved_value = std::move(moved_result).value();
    EXPECT_EQ(moved_value, 123);
}

TEST_F(ResultTest, Result_Success_ValueOrMethods) {
    // GIVEN: Success result
    auto result = Result<std::string>::success("success_value");
    
    // THEN: valueOr should return actual value
    EXPECT_EQ(result.valueOr("default"), "success_value");
    EXPECT_EQ(std::move(result).valueOr(std::string("default")), "success_value");
    
    // THEN: valueOrElse should return actual value
    auto lambda_result = Result<int>::success(42);
    int value = lambda_result.valueOrElse([]() { return 999; });
    EXPECT_EQ(value, 42);
}

//==============================================================================
// RED Phase: Result<T> Error Cases Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, Result_Error_CreatesErrorResult) {
    // WHEN: Creating error result
    auto result = Result<std::string>::error(sample_error_);
    
    // THEN: Result should represent error
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().message, sample_error_.message);
    EXPECT_EQ(result.error().category, sample_error_.category);
}

TEST_F(ResultTest, Result_Error_ExternalAuthorityError) {
    // GIVEN: External authority error
    auto external_error = ErrorInfo::externalAuthorityError(
        "Schema validation failed",
        "DAWProject Official",
        "XSD compliance violation");
    
    // WHEN: Creating error result with external authority error
    auto result = Result<int>::error(external_error);
    
    // THEN: Result should detect external authority error
    EXPECT_TRUE(result.isError());
    EXPECT_TRUE(result.isExternalAuthorityError());
    EXPECT_TRUE(result.error().isExternalAuthorityError());
}

TEST_F(ResultTest, Result_Error_ValueAccessThrows) {
    // GIVEN: Error result
    auto result = Result<std::string>::error(sample_error_);
    
    // THEN: Value access should throw
    EXPECT_THROW(result.value(), std::runtime_error);
    EXPECT_THROW(static_cast<const Result<std::string>&>(result).value(), std::runtime_error);
    EXPECT_THROW(std::move(result).value(), std::runtime_error);
}

TEST_F(ResultTest, Result_Error_ValueOrMethods) {
    // GIVEN: Error result
    auto result = Result<std::string>::error(sample_error_);
    
    // THEN: valueOr should return default value
    EXPECT_EQ(result.valueOr("default"), "default");
    EXPECT_EQ(std::move(result).valueOr(std::string("moved_default")), "moved_default");
    
    // THEN: valueOrElse should call default function
    auto lambda_result = Result<int>::error(sample_error_);
    int value = lambda_result.valueOrElse([]() { return 999; });
    EXPECT_EQ(value, 999);
}

TEST_F(ResultTest, Result_Success_ErrorAccessThrows) {
    // GIVEN: Success result
    auto result = Result<std::string>::success("test");
    
    // THEN: Error access should throw
    EXPECT_THROW(result.error(), std::runtime_error);
    EXPECT_THROW(static_cast<const Result<std::string>&>(result).error(), std::runtime_error);
    EXPECT_THROW(std::move(result).error(), std::runtime_error);
}

//==============================================================================
// RED Phase: Monadic Operations Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, Result_Map_SuccessCase) {
    // GIVEN: Success result
    auto result = Result<int>::success(5);
    
    // WHEN: Mapping with transformation function
    auto mapped = result.map([](int x) { return x * 2; });
    
    // THEN: Should transform success value
    EXPECT_TRUE(mapped.isSuccess());
    EXPECT_EQ(mapped.value(), 10);
}

TEST_F(ResultTest, Result_Map_ErrorCase) {
    // GIVEN: Error result
    auto result = Result<int>::error(sample_error_);
    
    // WHEN: Mapping with transformation function
    auto mapped = result.map([](int x) { return x * 2; });
    
    // THEN: Should preserve error
    EXPECT_TRUE(mapped.isError());
    EXPECT_EQ(mapped.error().message, sample_error_.message);
}

TEST_F(ResultTest, Result_FlatMap_SuccessCase) {
    // GIVEN: Success result
    auto result = Result<int>::success(5);
    
    // WHEN: FlatMapping with function that returns Result
    auto flat_mapped = result.flatMap([](int x) {
        return Result<std::string>::success("value: " + std::to_string(x));
    });
    
    // THEN: Should chain successfully
    EXPECT_TRUE(flat_mapped.isSuccess());
    EXPECT_EQ(flat_mapped.value(), "value: 5");
}

TEST_F(ResultTest, Result_FlatMap_ErrorInChain) {
    // GIVEN: Success result
    auto result = Result<int>::success(5);
    
    // WHEN: FlatMapping with function that returns error
    auto flat_mapped = result.flatMap([this](int x) {
        return Result<std::string>::error(sample_error_);
    });
    
    // THEN: Should propagate error from chain
    EXPECT_TRUE(flat_mapped.isError());
    EXPECT_EQ(flat_mapped.error().message, sample_error_.message);
}

TEST_F(ResultTest, Result_FlatMap_OriginalError) {
    // GIVEN: Error result
    auto result = Result<int>::error(sample_error_);
    
    // WHEN: FlatMapping 
    auto flat_mapped = result.flatMap([](int x) {
        return Result<std::string>::success("should not execute");
    });
    
    // THEN: Should preserve original error
    EXPECT_TRUE(flat_mapped.isError());
    EXPECT_EQ(flat_mapped.error().message, sample_error_.message);
}

TEST_F(ResultTest, Result_MapError_TransformsError) {
    // GIVEN: Error result
    auto result = Result<int>::error(sample_error_);
    
    // WHEN: Mapping error
    auto mapped = result.mapError([](const ErrorInfo& err) {
        auto new_error = err;
        new_error.message = "Transformed: " + err.message;
        return new_error;
    });
    
    // THEN: Should transform error
    EXPECT_TRUE(mapped.isError());
    EXPECT_THAT(mapped.error().message, HasSubstr("Transformed:"));
    EXPECT_THAT(mapped.error().message, HasSubstr(sample_error_.message));
}

TEST_F(ResultTest, Result_MapError_PreservesSuccess) {
    // GIVEN: Success result
    auto result = Result<int>::success(42);
    
    // WHEN: Mapping error (should not execute)
    auto mapped = result.mapError([](const ErrorInfo& err) {
        // This should never be called
        FAIL() << "mapError should not execute on success result";
        return err;
    });
    
    // THEN: Should preserve success
    EXPECT_TRUE(mapped.isSuccess());
    EXPECT_EQ(mapped.value(), 42);
}

//==============================================================================
// RED Phase: Result<void> Specialization Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, ResultVoid_Success_CreatesSuccessResult) {
    // WHEN: Creating void success result
    auto result = Result<void>::success();
    
    // THEN: Result should represent success
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_FALSE(result.isExternalAuthorityError());
}

TEST_F(ResultTest, ResultVoid_Error_CreatesErrorResult) {
    // WHEN: Creating void error result
    auto result = Result<void>::error(sample_error_);
    
    // THEN: Result should represent error
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().message, sample_error_.message);
}

TEST_F(ResultTest, ResultVoid_Map_SuccessCase) {
    // GIVEN: Void success result
    auto result = Result<void>::success();
    
    // WHEN: Mapping to value
    auto mapped = result.map([]() { return 42; });
    
    // THEN: Should create value result
    EXPECT_TRUE(mapped.isSuccess());
    EXPECT_EQ(mapped.value(), 42);
}

TEST_F(ResultTest, ResultVoid_Map_ErrorCase) {
    // GIVEN: Void error result
    auto result = Result<void>::error(sample_error_);
    
    // WHEN: Mapping to value (should not execute)
    auto mapped = result.map([]() { 
        FAIL() << "Map function should not execute on error result";
        return 42; 
    });
    
    // THEN: Should preserve error
    EXPECT_TRUE(mapped.isError());
    EXPECT_EQ(mapped.error().message, sample_error_.message);
}

TEST_F(ResultTest, ResultVoid_FlatMap_Chaining) {
    // GIVEN: Void success result
    auto result = Result<void>::success();
    
    // WHEN: FlatMapping with operation that returns Result
    auto flat_mapped = result.flatMap([]() {
        return Result<std::string>::success("chained_operation_success");
    });
    
    // THEN: Should chain successfully
    EXPECT_TRUE(flat_mapped.isSuccess());
    EXPECT_EQ(flat_mapped.value(), "chained_operation_success");
}

//==============================================================================
// RED Phase: External Authority Integration Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, ErrorInfo_ExternalAuthorityContext_DetectionMethods) {
    // GIVEN: External authority error
    auto external_error = ErrorInfo::externalAuthorityError(
        "External compliance failure",
        "DAWProject Schema v1.0",
        "XSD validation against external authority failed");
    
    // THEN: Detection methods should work correctly
    EXPECT_TRUE(external_error.isExternalAuthorityError());
    EXPECT_TRUE(external_error.externalAuthorityRelated);
    EXPECT_FALSE(external_error.externalSource->empty());
    
    // GIVEN: Non-external error
    auto internal_error = ErrorInfo::validationError("Internal validation", {}, "");
    
    // THEN: Should not be detected as external
    EXPECT_FALSE(internal_error.isExternalAuthorityError());
    EXPECT_FALSE(internal_error.externalAuthorityRelated);
}

TEST_F(ResultTest, Result_ExternalAuthorityError_PropagationInChains) {
    // GIVEN: External authority error
    auto external_error = ErrorInfo::externalAuthorityError(
        "Schema download failed",
        "GitHub DAWProject Repository",
        "HTTP 404 - Schema file not found");
    
    auto result = Result<int>::error(external_error);
    
    // WHEN: Chaining operations
    auto mapped = result.map([](int x) { return x * 2; });
    auto flat_mapped = result.flatMap([](int x) {
        return Result<std::string>::success("transformed");
    });
    
    // THEN: External authority context should be preserved
    EXPECT_TRUE(mapped.isExternalAuthorityError());
    EXPECT_TRUE(flat_mapped.isExternalAuthorityError());
    EXPECT_TRUE(mapped.error().isExternalAuthorityError());
    EXPECT_TRUE(flat_mapped.error().isExternalAuthorityError());
}

//==============================================================================
// RED Phase: Performance and Memory Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, Result_Performance_NoHeapAllocation) {
    // This test verifies that Result<T> doesn't allocate on the heap
    // for simple value types (important for performance-critical code)
    
    // Note: This is more of a static analysis requirement
    // In practice, we would verify this with profiling tools
    // For now, we test basic functionality that should be efficient
    
    const int iterations = 10000;
    
    // WHEN: Creating many success results
    for (int i = 0; i < iterations; ++i) {
        auto result = Result<int>::success(i);
        EXPECT_TRUE(result.isSuccess());
        EXPECT_EQ(result.value(), i);
    }
    
    // WHEN: Creating many error results
    for (int i = 0; i < iterations; ++i) {
        auto error = ErrorInfo::validationError("test error " + std::to_string(i));
        auto result = Result<int>::error(error);
        EXPECT_TRUE(result.isError());
    }
}

TEST_F(ResultTest, Result_MoveSemantics_NoUnnecessaryCopying) {
    // GIVEN: Large string value (to test move semantics)
    std::string large_string(10000, 'x');
    std::string original = large_string;
    
    // WHEN: Creating result with move
    auto result = Result<std::string>::success(std::move(large_string));
    
    // THEN: Original string should be moved (empty)
    EXPECT_TRUE(large_string.empty());  // Moved from
    EXPECT_EQ(result.value(), original);
    
    // WHEN: Moving value out of result
    std::string extracted = std::move(result).value();
    
    // THEN: Value should be moved successfully  
    EXPECT_EQ(extracted, original);
}

//==============================================================================
// RED Phase: Thread Safety Tests (Should FAIL initially)
//==============================================================================

TEST_F(ResultTest, Result_ThreadSafety_ImmutableAfterConstruction) {
    // GIVEN: Result instances
    auto success_result = Result<int>::success(42);
    auto error_result = Result<int>::error(sample_error_);
    
    // Multiple threads should be able to safely read from the same Result
    // This is a basic test - in practice we'd use thread sanitizer
    
    const int num_threads = 4;
    const int iterations_per_thread = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    
    // WHEN: Multiple threads access the same results concurrently
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations_per_thread; ++i) {
                if (success_result.isSuccess()) {
                    success_count++;
                }
                if (error_result.isError()) {
                    error_count++;
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // THEN: All accesses should have been successful
    EXPECT_EQ(success_count.load(), num_threads * iterations_per_thread);
    EXPECT_EQ(error_count.load(), num_threads * iterations_per_thread);
}

//==============================================================================
// RED Phase: Error Information Utility Tests (Should FAIL initially) 
//==============================================================================

TEST_F(ResultTest, ErrorInfo_FormattedMessage_IncludesContext) {
    // GIVEN: Error with rich context
    auto error = ErrorInfo::networkError(
        "Connection timeout",
        "https://external-authority.com/schema.xsd",
        "Timeout after 30 seconds");
    error.operation = "downloadExternalSchema";
    error.component = "ExternalSchemaManager";
    
    // WHEN: Getting formatted message
    std::string basic_message = error.getFormattedMessage(false);
    std::string detailed_message = error.getFormattedMessage(true);
    
    // THEN: Messages should include appropriate context
    EXPECT_THAT(basic_message, HasSubstr("Connection timeout"));
    EXPECT_THAT(detailed_message, HasSubstr("Connection timeout"));
    EXPECT_THAT(detailed_message, HasSubstr("ExternalSchemaManager"));
    EXPECT_THAT(detailed_message, HasSubstr("downloadExternalSchema"));
    EXPECT_GT(detailed_message.length(), basic_message.length());
}

TEST_F(ResultTest, ErrorInfo_ErrorChaining_GetAllErrors) {
    // GIVEN: Chain of errors (cause -> error -> related)
    auto root_cause = ErrorInfo::networkError("Network unreachable");
    auto intermediate_error = ErrorInfo::externalAuthorityError(
        "Schema download failed", "DAWProject", "Network issue");
    intermediate_error.cause = std::make_shared<ErrorInfo>(root_cause);
    
    auto final_error = ErrorInfo::validationError("Validation incomplete");
    final_error.cause = std::make_shared<ErrorInfo>(intermediate_error);
    
    auto related_error = ErrorInfo::configurationError("Cache config invalid", "CacheManager");
    final_error.relatedErrors.push_back(related_error);
    
    // WHEN: Getting all errors
    auto all_errors = final_error.getAllErrors();
    
    // THEN: Should include entire error chain
    EXPECT_GE(all_errors.size(), 4);  // final + intermediate + root + related
    
    // Verify error messages are present
    std::vector<std::string> messages;
    for (const auto& err : all_errors) {
        messages.push_back(err.message);
    }
    
    EXPECT_THAT(messages, Contains("Validation incomplete"));
    EXPECT_THAT(messages, Contains("Schema download failed"));
    EXPECT_THAT(messages, Contains("Network unreachable"));
    EXPECT_THAT(messages, Contains("Cache config invalid"));
}