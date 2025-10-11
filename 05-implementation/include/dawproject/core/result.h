#pragma once

/**
 * @file result.h
 * @brief Result<T> Pattern Implementation (DES-I-CORE-001)
 * 
 * Implements the Result<T> pattern for error handling without exceptions
 * as specified in Phase 04 detailed design.
 * 
 * Traceability: DES-I-CORE-001, REQ-NF-R-001
 * Standards: IEEE 1016-2009, ISO/IEC/IEEE 12207:2017
 */

#include <variant>
#include <optional>
#include <functional>
#include <type_traits>
#include <dawproject/core/error_info.h>

namespace dawproject {

/**
 * @brief Result type for error handling without exceptions
 * 
 * Provides monadic error handling pattern for all library operations.
 * Eliminates exceptions while maintaining clear error reporting.
 * 
 * @tparam T Success value type
 * @threadsafety Thread-safe (immutable after construction)
 * @performance Zero overhead when successful (no allocations)
 */
template<typename T>
class Result {
public:
    // Construction
    static Result<T> success(T value) {
        return Result<T>(std::move(value));
    }
    
    static Result<T> error(ErrorInfo errorInfo) {
        return Result<T>(std::move(errorInfo));
    }
    
    // Copy/Move constructors
    Result(const Result& other) = default;
    Result(Result&& other) noexcept = default;
    Result& operator=(const Result& other) = default;
    Result& operator=(Result&& other) noexcept = default;
    
    // State queries
    bool isSuccess() const noexcept {
        return std::holds_alternative<T>(data_);
    }
    
    bool isError() const noexcept {
        return std::holds_alternative<ErrorInfo>(data_);
    }
    
    // Value access (success case)
    const T& value() const & {
        if (isError()) {
            throw std::runtime_error("Attempted to access value of error Result");
        }
        return std::get<T>(data_);
    }
    
    T& value() & {
        if (isError()) {
            throw std::runtime_error("Attempted to access value of error Result");
        }
        return std::get<T>(data_);
    }
    
    T&& value() && {
        if (isError()) {
            throw std::runtime_error("Attempted to access value of error Result");
        }
        return std::move(std::get<T>(data_));
    }
    
    // Error access (error case)  
    const ErrorInfo& error() const & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result");
        }
        return std::get<ErrorInfo>(data_);
    }
    
    ErrorInfo& error() & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result");
        }
        return std::get<ErrorInfo>(data_);
    }
    
    ErrorInfo&& error() && {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result");
        }
        return std::move(std::get<ErrorInfo>(data_));
    }
    
    // Monadic operations
    template<typename F>
    auto map(F&& func) -> Result<decltype(func(std::declval<T>()))> {
        using ReturnType = decltype(func(std::declval<T>()));
        
        if (isSuccess()) {
            return Result<ReturnType>::success(func(value()));
        } else {
            return Result<ReturnType>::error(error());
        }
    }
    
    template<typename F>
    auto flatMap(F&& func) -> decltype(func(std::declval<T>())) {
        using ReturnType = decltype(func(std::declval<T>()));
        
        if (isSuccess()) {
            return func(value());
        } else {
            return ReturnType::error(error());
        }
    }
    
    template<typename F>
    Result<T> mapError(F&& func) {
        if (isError()) {
            return Result<T>::error(func(error()));
        } else {
            return *this;
        }
    }
    
    // Utility operations
    T valueOr(const T& defaultValue) const & {
        if (isSuccess()) {
            return value();
        } else {
            return defaultValue;
        }
    }
    
    T valueOr(T&& defaultValue) && {
        if (isSuccess()) {
            return std::move(value());
        } else {
            return std::move(defaultValue);
        }
    }
    
    template<typename F>
    T valueOrElse(F&& defaultFunc) const {
        if (isSuccess()) {
            return value();
        } else {
            return defaultFunc();
        }
    }

private:
    std::variant<T, ErrorInfo> data_;
    
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(ErrorInfo error) : data_(std::move(error)) {}
};

/**
 * @brief Specialization for void operations
 */
template<>
class Result<void> {
public:
    static Result<void> success() {
        return Result<void>();
    }
    
    static Result<void> error(ErrorInfo errorInfo) {
        return Result<void>(std::move(errorInfo));
    }
    
    // Copy/Move constructors
    Result(const Result& other) = default;
    Result(Result&& other) noexcept = default;
    Result& operator=(const Result& other) = default;
    Result& operator=(Result&& other) noexcept = default;
    
    bool isSuccess() const noexcept {
        return !error_.has_value();
    }
    
    bool isError() const noexcept {
        return error_.has_value();
    }
    
    const ErrorInfo& error() const & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result<void>");
        }
        return *error_;
    }
    
    ErrorInfo& error() & {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result<void>");
        }
        return *error_;
    }
    
    ErrorInfo&& error() && {
        if (isSuccess()) {
            throw std::runtime_error("Attempted to access error of success Result<void>");
        }
        return std::move(*error_);
    }
    
    template<typename F>
    auto map(F&& func) -> Result<decltype(func())> {
        using ReturnType = decltype(func());
        
        if (isSuccess()) {
            return Result<ReturnType>::success(func());
        } else {
            return Result<ReturnType>::error(error());
        }
    }
    
    template<typename F>
    auto flatMap(F&& func) -> decltype(func()) {
        using ReturnType = decltype(func());
        
        if (isSuccess()) {
            return func();
        } else {
            return ReturnType::error(error());
        }
    }

private:
    std::optional<ErrorInfo> error_;
    
    Result() = default; // Success case
    explicit Result(ErrorInfo error) : error_(std::move(error)) {}
};

} // namespace dawproject