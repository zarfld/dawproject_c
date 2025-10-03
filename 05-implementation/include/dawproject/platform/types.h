#pragma once

/**
 * @file types.h
 * @brief Common types and forward declarations for Platform Abstraction Layer
 */

#include <chrono>
#include <cstdint>
#include <string>

namespace dawproject::platform {
    
    // Type aliases for common usage
    using ThreadId = uint64_t;
    using Timestamp = std::chrono::system_clock::time_point;
    
} // namespace dawproject::platform