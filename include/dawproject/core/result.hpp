//==============================================================================
// DAWProject C++ Library - Phase 05 Implementation
// Result<T> Pattern Header - Core Error Handling Interface
// 
// Standards Compliance:
//   - ISO/IEC/IEEE 12207:2017 Implementation Process  
//   - XP Practices: Simple Design, Test-First Development
//   - IEEE 1016-2009 Design Specification: DES-I-CORE-001
//
// External Authority Compliance: CRITICAL
//   This component provides error handling for external authority operations
//==============================================================================

#pragma once

#include <variant>
#include <optional>
#include <functional>
#include <type_traits>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

namespace dawproject {

//==============================================================================
// Forward Declarations
//==============================================================================

struct ErrorInfo;
template<typename T> class Result;

//==============================================================================
// Error Information Structure (DES-I-CORE-001)
//==============================================================================

/**
 * @brief Comprehensive error information for all library operations
 * 
 * Provides structured error information with external authority compliance
 * context, performance metrics, and actionable recovery information.
 * 
 * @threadsafety Immutable after construction - thread-safe
 * @performance Minimal overhead - prefer over exceptions
 * @external_authority Includes compliance context for external operations
 */
struct ErrorInfo {
    /**
     * @brief Error category for systematic error handling
     */
    enum class Category {
        // Network and external authority errors (CRITICAL)
        NetworkError,           ///< HTTP, connectivity issues  
        ExternalAuthorityError, ///< Schema access, compliance violations
        
        // Validation and parsing errors
        ValidationError,        ///< XSD validation failures
        ParseError,            ///< XML/JSON parsing issues
        
        // File system and I/O errors
        FileSystemError,       ///< File access, permissions
        CacheError,           ///< Cache management issues
        
        // Configuration and setup errors  
        ConfigurationError,    ///< Invalid configuration
        DependencyError,      ///< Missing dependencies (libxml2, libcurl)
        
        // Runtime and system errors
        MemoryError,          ///< Allocation failures
        TimeoutError,         ///< Operation timeouts
        ConcurrencyError,     ///< Thread safety violations
        
        // Business logic errors
        InvalidOperation,     ///< Unsupported operations
        InvalidData,         ///< Data integrity issues
        
        // Unknown or system errors
        UnknownError         ///< Catch-all for unexpected issues
    };
    
    /**
     * @brief Error severity for prioritization and handling
     */
    enum class Severity {
        Info,       ///< Informational - operation can continue
        Warning,    ///< Warning - operation succeeded with issues
        Error,      ///< Error - operation failed but recoverable  
        Critical,   ///< Critical - operation failed, system integrity at risk
        Fatal       ///< Fatal - operation failed, immediate shutdown required
    };
    
    // Core error information
    Category category;
    Severity severity;
    std::string code;           ///< Machine-readable error code (e.g., "EXT_SCHEMA_001")
    std::string message;        ///< Human-readable error description
    std::string details;        ///< Detailed technical information
    
    // Contextual information
    std::optional<std::string> operation;      ///< Operation that failed
    std::optional<std::string> component;      ///< Component where error occurred
    std::optional<std::filesystem::path> filePath; ///< Related file path
    std::optional<std::string> url;           ///< Related URL (for network errors)
    
    // External authority context (CRITICAL for compliance)
    std::optional<std::string> externalSource; ///< External authority source
    std::optional<std::string> schemaVersion;  ///< Schema version related to error
    bool externalAuthorityRelated = false;     ///< Error related to external authority
    
    // Recovery and action information
    std::vector<std::string> suggestedActions; ///< User-actionable recovery steps
    std::vector<std::string> technicalActions; ///< Developer-actionable steps
    bool isRecoverable = true;                 ///< Whether error is recoverable
    
    // Debugging and performance information
    std::chrono::system_clock::time_point timestamp; ///< When error occurred
    std::optional<std::chrono::milliseconds> duration; ///< Operation duration
    std::optional<size_t> memoryUsage;        ///< Memory usage at time of error
    std::optional<std::string> stackTrace;    ///< Stack trace (debug builds)
    
    // Chained error support
    std::shared_ptr<ErrorInfo> cause;         ///< Underlying cause (if any)
    std::vector<ErrorInfo> relatedErrors;     ///< Related errors
    
    //==========================================================================
    // Factory Methods for Common Error Types
    //==========================================================================
    
    /**
     * @brief Create network error with external authority context
     * @param message Human-readable error message
     * @param url URL that caused the error (optional)
     * @param details Technical details (optional)
     * @return ErrorInfo Network error instance
     */
    static ErrorInfo networkError(
        const std::string& message,
        const std::string& url = "",
        const std::string& details = "");
    
    /**
     * @brief Create external authority error (CRITICAL)
     * @param message Human-readable error message
     * @param externalSource External authority source identifier
     * @param details Technical details (optional)
     * @return ErrorInfo External authority error instance
     */
    static ErrorInfo externalAuthorityError(
        const std::string& message,
        const std::string& externalSource,
        const std::string& details = "");
    
    /**
     * @brief Create validation error with file context
     * @param message Human-readable error message
     * @param filePath File path related to error (optional)
     * @param details Technical details (optional)
     * @return ErrorInfo Validation error instance
     */
    static ErrorInfo validationError(
        const std::string& message,
        const std::filesystem::path& filePath = {},
        const std::string& details = "");
    
    /**
     * @brief Create validation error (simple overload)
     * @param message Human-readable error message
     * @return ErrorInfo Validation error instance
     */
    static ErrorInfo validationError(const std::string& message);
    
    /**
     * @brief Create configuration error with recovery actions
     * @param message Human-readable error message
     * @param component Component with configuration error
     * @param details Technical details (optional)
     * @return ErrorInfo Configuration error instance
     */
    static ErrorInfo configurationError(
        const std::string& message,
        const std::string& component,
        const std::string& details = "");
    
    /**
     * @brief Create dependency error with installation instructions
     * @param dependency Name of missing dependency
     * @param details Technical details (optional)
     * @param installInstructions Installation steps (optional)
     * @return ErrorInfo Dependency error instance
     */
    static ErrorInfo dependencyError(
        const std::string& dependency,
        const std::string& details = "",
        const std::vector<std::string>& installInstructions = {});
        
    //==========================================================================
    // Utility Methods
    //==========================================================================
    
    /**
     * @brief Check if error is related to external authority operations
     * @return true if error involves external authority compliance
     */
    bool isExternalAuthorityError() const noexcept;
    
    /**
     * @brief Check if error is recoverable
     * @return true if automatic or manual recovery is possible
     */
    bool canRecover() const noexcept;
    
    /**
     * @brief Get formatted error message with context
     * @param includeDetails Include technical details in output
     * @return Formatted error message string
     */
    std::string getFormattedMessage(bool includeDetails = false) const;
    
    /**
     * @brief Get all errors in the chain (this + cause + related)
     * @return Vector of all related ErrorInfo instances
     */
    std::vector<ErrorInfo> getAllErrors() const;
    
private:
    /**
     * @brief Generate unique error code for error instance
     * @param prefix Error category prefix
     * @param category Error category enum
     * @return Unique error code string
     */
    static std::string generateErrorCode(const std::string& prefix, Category category);
};

//==============================================================================
// Result<T> Pattern Implementation (DES-I-CORE-001)  
//==============================================================================

/**
 * @brief Monadic error handling type for all library operations
 * 
 * Provides type-safe error handling without exceptions, following
 * functional programming patterns with external authority compliance
 * tracking.
 * 
 * @tparam T Success value type
 * @threadsafety Thread-safe (immutable after construction)
 * @performance Zero overhead when successful (no allocations)
 * @external_authority Tracks external authority operation context
 */
template<typename T>
class Result {
private:
    std::variant<T, ErrorInfo> data_;
    
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(ErrorInfo error) : data_(std::move(error)) {}

public:
    //==========================================================================
    // Construction (Static Factory Methods)
    //==========================================================================
    
    /**
     * @brief Create successful result with value
     * @param value Success value to wrap
     * @return Result<T> Success instance
     */
    static Result<T> success(T value) {
        return Result<T>(std::move(value));
    }
    
    /**
     * @brief Create error result with detailed error information
     * @param errorInfo Comprehensive error details
     * @return Result<T> Error instance  
     */
    static Result<T> error(ErrorInfo errorInfo) {
        return Result<T>(std::move(errorInfo));
    }
    
    //==========================================================================
    // State Query Methods
    //==========================================================================
    
    /**
     * @brief Check if result represents success
     * @return true if result contains value, false if error
     */
    bool isSuccess() const noexcept {
        return std::holds_alternative<T>(data_);
    }
    
    /**
     * @brief Check if result represents error  
     * @return true if result contains error, false if success
     */
    bool isError() const noexcept {
        return std::holds_alternative<ErrorInfo>(data_);
    }
    
    /**
     * @brief Check if result represents external authority error
     * @return true if error is related to external authority operations
     */
    bool isExternalAuthorityError() const noexcept {
        if (isError()) {
            return std::get<ErrorInfo>(data_).isExternalAuthorityError();
        }
        return false;
    }
    
    //==========================================================================
    // Value Access Methods (Success Case)
    //==========================================================================
    
    /**
     * @brief Get value (const lvalue reference)
     * @return const T& Value reference
     * @throws std::runtime_error if result is error
     */
    const T& value() const & {
        if (isError()) {
            throw std::runtime_error("Attempted to get value from error result: " + 
                                   std::get<ErrorInfo>(data_).message);
        }
        return std::get<T>(data_);
    }
    
    /**
     * @brief Get value (mutable lvalue reference)
     * @return T& Mutable value reference
     * @throws std::runtime_error if result is error
     */
    T& value() & {
        if (isError()) {
            throw std::runtime_error("Attempted to get value from error result: " + 
                                   std::get<ErrorInfo>(data_).message);
        }
        return std::get<T>(data_);
    }
    
    /**
     * @brief Get value (rvalue reference - moves value out)
     * @return T&& Rvalue reference to value
     * @throws std::runtime_error if result is error
     */
    T&& value() && {
        if (isError()) {
            throw std::runtime_error("Attempted to get value from error result: " + 
                                   std::get<ErrorInfo>(data_).message);
        }
        return std::move(std::get<T>(data_));
    }
    
    //==========================================================================
    // Error Access Methods (Error Case)
    //==========================================================================
    
    /**
     * @brief Get error (const lvalue reference)
     * @return const ErrorInfo& Error information reference
     * @throws std::runtime_error if result is success
     */
    const ErrorInfo& error() const & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return std::get<ErrorInfo>(data_);
    }
    
    /**
     * @brief Get error (mutable lvalue reference)
     * @return ErrorInfo& Mutable error reference
     * @throws std::runtime_error if result is success
     */
    ErrorInfo& error() & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return std::get<ErrorInfo>(data_);
    }
    
    /**
     * @brief Get error (rvalue reference - moves error out)
     * @return ErrorInfo&& Rvalue reference to error
     * @throws std::runtime_error if result is success
     */
    ErrorInfo&& error() && {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return std::move(std::get<ErrorInfo>(data_));
    }
    
    //==========================================================================
    // Monadic Operations (Functional Programming Style)
    //==========================================================================
    
    /**
     * @brief Transform success value while preserving errors (functor map)
     * @tparam F Function type: T -> U
     * @param func Transformation function
     * @return Result<U> Transformed result or original error
     */
    template<typename F>
    auto map(F&& func) -> Result<decltype(func(std::declval<T>()))> {
        using U = decltype(func(std::declval<T>()));
        
        if (isSuccess()) {
            return Result<U>::success(func(std::get<T>(data_)));
        } else {
            return Result<U>::error(std::get<ErrorInfo>(data_));
        }
    }
    
    /**
     * @brief Chain operations that return Result (monadic bind/flatMap)
     * @tparam F Function type: T -> Result<U>
     * @param func Chaining function that returns Result
     * @return Result<U> Result of chained operation or original error
     */
    template<typename F>
    auto flatMap(F&& func) -> decltype(func(std::declval<T>())) {
        if (isSuccess()) {
            return func(std::get<T>(data_));
        } else {
            using ReturnType = decltype(func(std::declval<T>()));
            return ReturnType::error(std::get<ErrorInfo>(data_));
        }
    }
    
    /**
     * @brief Transform error while preserving success values
     * @tparam F Function type: ErrorInfo -> ErrorInfo
     * @param func Error transformation function
     * @return Result<T> Original success or transformed error
     */
    template<typename F>
    Result<T> mapError(F&& func) {
        if (isError()) {
            return Result<T>::error(func(std::get<ErrorInfo>(data_)));
        } else {
            return Result<T>::success(std::get<T>(data_));
        }
    }
    
    //==========================================================================
    // Utility Operations
    //==========================================================================
    
    /**
     * @brief Get value or return default (const lvalue version)
     * @param defaultValue Default value to return if error
     * @return T Value if success, defaultValue if error
     */
    T valueOr(const T& defaultValue) const & {
        if (isSuccess()) {
            return std::get<T>(data_);
        } else {
            return defaultValue;
        }
    }
    
    /**
     * @brief Get value or return default (rvalue version)
     * @param defaultValue Default value to return if error
     * @return T Value if success, moved defaultValue if error
     */
    T valueOr(T&& defaultValue) && {
        if (isSuccess()) {
            return std::move(std::get<T>(data_));
        } else {
            return std::move(defaultValue);
        }
    }
    
    /**
     * @brief Get value or compute default using function
     * @tparam F Function type: void -> T
     * @param defaultFunc Function to compute default value
     * @return T Value if success, result of defaultFunc() if error
     */
    template<typename F>
    T valueOrElse(F&& defaultFunc) const {
        if (isSuccess()) {
            return std::get<T>(data_);
        } else {
            return defaultFunc();
        }
    }
    
    //==========================================================================
    // External Authority Compliance Methods
    //==========================================================================
    
    /**
     * @brief Check if operation involved external authority
     * @return true if success/error involved external authority operations
     */
    bool hasExternalAuthorityContext() const noexcept {
        if (isError()) {
            return std::get<ErrorInfo>(data_).externalAuthorityRelated;
        }
        // For success cases, we can't determine this from Result alone
        // This would need to be tracked by the calling code
        return false;
    }
};

//==============================================================================
// Result<void> Specialization (DES-I-CORE-001)
//==============================================================================

/**
 * @brief Specialization for operations that don't return values
 * 
 * Provides error handling for void operations while maintaining
 * the same monadic interface patterns.
 */
template<>
class Result<void> {
private:
    std::optional<ErrorInfo> error_;
    
    Result() = default; // Success case
    explicit Result(ErrorInfo error) : error_(std::move(error)) {}

public:
    //==========================================================================
    // Construction
    //==========================================================================
    
    /**
     * @brief Create successful void result
     * @return Result<void> Success instance
     */
    static Result<void> success() {
        return Result<void>();
    }
    
    /**
     * @brief Create error result with detailed error information
     * @param errorInfo Comprehensive error details
     * @return Result<void> Error instance
     */
    static Result<void> error(ErrorInfo errorInfo) {
        return Result<void>(std::move(errorInfo));
    }
    
    //==========================================================================
    // State Query Methods
    //==========================================================================
    
    /**
     * @brief Check if result represents success
     * @return true if no error, false if error
     */
    bool isSuccess() const noexcept {
        return !error_.has_value();
    }
    
    /**
     * @brief Check if result represents error
     * @return true if error, false if success
     */
    bool isError() const noexcept {
        return error_.has_value();
    }
    
    /**
     * @brief Check if result represents external authority error
     * @return true if error is related to external authority operations
     */
    bool isExternalAuthorityError() const noexcept {
        if (isError()) {
            return error_->isExternalAuthorityError();
        }
        return false;
    }
    
    //==========================================================================
    // Error Access Methods
    //==========================================================================
    
    /**
     * @brief Get error (const lvalue reference)
     * @return const ErrorInfo& Error information reference
     * @throws std::runtime_error if result is success
     */
    const ErrorInfo& error() const & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return *error_;
    }
    
    /**
     * @brief Get error (mutable lvalue reference)
     * @return ErrorInfo& Mutable error reference
     * @throws std::runtime_error if result is success
     */
    ErrorInfo& error() & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return *error_;
    }
    
    /**
     * @brief Get error (rvalue reference - moves error out)
     * @return ErrorInfo&& Rvalue reference to error
     * @throws std::runtime_error if result is success
     */
    ErrorInfo&& error() && {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return std::move(*error_);
    }
    
    //==========================================================================
    // Monadic Operations
    //==========================================================================
    
    /**
     * @brief Transform void success to value (functor map)
     * @tparam F Function type: void -> U
     * @param func Function to call on success
     * @return Result<U> Function result or original error
     */
    template<typename F>
    auto map(F&& func) -> Result<decltype(func())> {
        using U = decltype(func());
        
        if (isSuccess()) {
            return Result<U>::success(func());
        } else {
            return Result<U>::error(*error_);
        }
    }
    
    /**
     * @brief Chain void operations that return Result (monadic bind)
     * @tparam F Function type: void -> Result<U>
     * @param func Chaining function that returns Result
     * @return Result<U> Result of chained operation or original error
     */
    template<typename F>
    auto flatMap(F&& func) -> decltype(func()) {
        if (isSuccess()) {
            return func();
        } else {
            using ReturnType = decltype(func());
            return ReturnType::error(*error_);
        }
    }
    
    /**
     * @brief Transform error while preserving success
     * @tparam F Function type: ErrorInfo -> ErrorInfo
     * @param func Error transformation function
     * @return Result<void> Original success or transformed error
     */
    template<typename F>
    Result<void> mapError(F&& func) {
        if (isError()) {
            return Result<void>::error(func(*error_));
        } else {
            return Result<void>::success();
        }
    }
};

} // namespace dawproject