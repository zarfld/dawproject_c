/**
 * @file memory_impl.h
 * @brief Minimal Memory Manager Implementation for TDD GREEN Phase
 * 
 * Implements IMemoryManager interface with minimal functionality to make tests pass.
 * Following XP Simple Design: write only enough code to pass the tests.
 */

#pragma once

#include "dawproject/platform/interfaces.h"
#include "dawproject/platform/factory.h"
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace dawproject::platform {

/**
 * @brief Minimal memory manager implementation using standard library
 * 
 * This is the simplest possible implementation that satisfies the interface contract.
 * It will be refactored and improved in subsequent TDD cycles.
 */
class MemoryManagerImpl : public IMemoryManager {
public:
    MemoryManagerImpl() : totalAllocated_(0), peakAllocated_(0) {}
    virtual ~MemoryManagerImpl() = default;

    void* allocate(size_t size, size_t alignment = sizeof(void*)) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        void* ptr = nullptr;
        
        if (alignment <= sizeof(void*)) {
            // Standard allocation
            ptr = std::malloc(size);
            if (ptr != nullptr) {
                allocations_[ptr] = size;
            }
        } else {
            // Aligned allocation using platform-specific methods
#ifdef _WIN32
            ptr = _aligned_malloc(size, alignment);
#else
            // POSIX aligned allocation
            if (posix_memalign(&ptr, alignment, size) != 0) {
                ptr = nullptr;
            }
#endif
            if (ptr != nullptr) {
                alignedAllocations_[ptr] = size;
            }
        }
        
        if (ptr != nullptr) {
            totalAllocated_ += size;
            peakAllocated_ = std::max(peakAllocated_, totalAllocated_);
        }
        
        return ptr;
    }

    void deallocate(void* ptr) override {
        if (ptr == nullptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check if it's a regular allocation
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            totalAllocated_ -= it->second;
            allocations_.erase(it);
            std::free(ptr);
            return;
        }
        
        // Check if it's an aligned allocation
        auto ait = alignedAllocations_.find(ptr);
        if (ait != alignedAllocations_.end()) {
            totalAllocated_ -= ait->second;
            alignedAllocations_.erase(ait);
#ifdef _WIN32
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
            return;
        }
        
        // If we get here, the pointer wasn't found in our tracking
        // This could be a double-delete or invalid pointer
        // In debug mode, we might want to assert or log this
    }

    void* reallocate(void* ptr, size_t newSize, size_t alignment = sizeof(void*)) override {
        if (ptr == nullptr) {
            return allocate(newSize, alignment);
        }
        
        if (newSize == 0) {
            deallocate(ptr);
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Find original size and determine if it was aligned or regular allocation
        size_t oldSize = 0;
        bool wasAligned = false;
        
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            oldSize = it->second;
            wasAligned = false;
        } else {
            auto ait = alignedAllocations_.find(ptr);
            if (ait != alignedAllocations_.end()) {
                oldSize = ait->second;
                wasAligned = true;
            }
        }
        
        void* newPtr = nullptr;
        
        if (alignment <= sizeof(void*) && !wasAligned) {
            // Standard reallocation for regular memory
            newPtr = std::realloc(ptr, newSize);
            if (newPtr != nullptr) {
                // Update tracking
                totalAllocated_ -= oldSize;
                allocations_.erase(allocations_.find(ptr));
                
                allocations_[newPtr] = newSize;
                totalAllocated_ += newSize;
                peakAllocated_ = std::max(peakAllocated_, totalAllocated_);
            }
        } else {
            // For aligned reallocation or changing alignment, allocate new and copy
            // Release the lock temporarily for the allocate call to avoid deadlock
            mutex_.unlock();
            newPtr = allocate(newSize, alignment);
            mutex_.lock();
            
            if (newPtr != nullptr && oldSize > 0) {
                std::memcpy(newPtr, ptr, std::min(oldSize, newSize));
                
                // Manually handle old pointer cleanup to avoid deallocate recursion
                if (wasAligned) {
                    totalAllocated_ -= oldSize;
                    alignedAllocations_.erase(alignedAllocations_.find(ptr));
#ifdef _WIN32
                    _aligned_free(ptr);
#else
                    std::free(ptr);
#endif
                } else {
                    totalAllocated_ -= oldSize;
                    allocations_.erase(allocations_.find(ptr));
                    std::free(ptr);
                }
            }
        }
        
        return newPtr;
    }

    void* allocateAligned(size_t size, size_t alignment) override {
        void* ptr = nullptr;
        
#ifdef _WIN32
        ptr = _aligned_malloc(size, alignment);
#else
        if (posix_memalign(&ptr, alignment, size) != 0) {
            ptr = nullptr;
        }
#endif
        
        if (ptr != nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            alignedAllocations_[ptr] = size;
            totalAllocated_ += size;
            peakAllocated_ = std::max(peakAllocated_, totalAllocated_);
        }
        
        return ptr;
    }

    void deallocateAligned(void* ptr) override {
        if (ptr == nullptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = alignedAllocations_.find(ptr);
        if (it != alignedAllocations_.end()) {
            totalAllocated_ -= it->second;
            alignedAllocations_.erase(it);
        }
        
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    size_t getAllocatedSize(void* ptr) const override {
        if (ptr == nullptr) return 0;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check regular allocations
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            return it->second;
        }
        
        // Check aligned allocations
        auto ait = alignedAllocations_.find(ptr);
        if (ait != alignedAllocations_.end()) {
            return ait->second;
        }
        
        return 0;
    }

    size_t getTotalAllocated() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return totalAllocated_;
    }

    size_t getPeakAllocated() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return peakAllocated_;
    }

    bool validateHeap() const override {
        // Simple validation - always return true for minimal implementation
        // Real implementation would check heap integrity
        return true;
    }

    void reportLeaks() const override {
        // Simple leak reporting - no-op for minimal implementation
        // Real implementation would report active allocations
        std::lock_guard<std::mutex> lock(mutex_);
        // Could log allocations_.size() + alignedAllocations_.size() leaks
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<void*, size_t> allocations_;
    std::unordered_map<void*, size_t> alignedAllocations_;
    size_t totalAllocated_;
    size_t peakAllocated_;
};

} // namespace dawproject::platform