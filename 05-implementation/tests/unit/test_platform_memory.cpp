/**
 * @file test_platform_memory.cpp  
 * @brief RED Phase Tests for Platform Memory Manager Interface
 * 
 * This file contains failing tests that drive the implementation of IMemoryManager interface.
 * Following TDD methodology: RED → GREEN → REFACTOR
 * 
 * Test Coverage:
 * - Memory manager factory creation and validation
 * - Basic memory allocation and deallocation
 * - Aligned memory allocation and deallocation  
 * - Memory reallocation operations
 * - Memory tracking and statistics
 * - Memory validation and leak detection
 * - Error handling and edge cases
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include "dawproject/platform/factory.h"
#include "dawproject/platform/interfaces.h"
#include <vector>
#include <cstring>

using namespace dawproject::platform;

/**
 * @brief Test fixture for memory manager operations
 */
class MemoryTestFixture {
public:
    void SetUp() {
        factory_ = &PlatformFactory::getInstance();
        REQUIRE(factory_ != nullptr);
        
        memoryManager_ = factory_->createMemoryManager();
        // RED: This should initially fail
        REQUIRE(memoryManager_ != nullptr);
    }

protected:
    PlatformFactory* factory_;
    std::unique_ptr<IMemoryManager> memoryManager_;
};

TEST_CASE_METHOD(MemoryTestFixture, "Memory Manager Factory Creation", "[memory][factory]") {
    SetUp();
    
    SECTION("Factory creates valid memory manager instance") {
        // RED: This should fail initially
        REQUIRE(memoryManager_ != nullptr);
        
        // Verify memory manager instance type is correct
        REQUIRE(dynamic_cast<IMemoryManager*>(memoryManager_.get()) != nullptr);
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Basic Memory Allocation", "[memory][allocation]") {
    SetUp();
    
    SECTION("allocate() returns valid memory") {
        // RED: This drives basic allocation implementation
        void* ptr = memoryManager_->allocate(1024);
        REQUIRE(ptr != nullptr);
        
        // Memory should be usable
        std::memset(ptr, 0x42, 1024);
        REQUIRE(static_cast<char*>(ptr)[0] == 0x42);
        REQUIRE(static_cast<char*>(ptr)[1023] == 0x42);
        
        // Clean up
        memoryManager_->deallocate(ptr);
    }
    
    SECTION("allocate() with different sizes works") {
        // RED: This drives size handling
        std::vector<void*> allocations;
        std::vector<size_t> sizes = {1, 16, 64, 256, 1024, 4096};
        
        for (size_t size : sizes) {
            void* ptr = memoryManager_->allocate(size);
            REQUIRE(ptr != nullptr);
            allocations.push_back(ptr);
            
            // Test memory is writable
            std::memset(ptr, static_cast<int>(size & 0xFF), size);
        }
        
        // Clean up all allocations
        for (void* ptr : allocations) {
            memoryManager_->deallocate(ptr);
        }
    }
    
    SECTION("deallocate() handles valid pointers") {
        // RED: This drives deallocation implementation
        void* ptr = memoryManager_->allocate(512);
        REQUIRE(ptr != nullptr);
        
        // Deallocation should not throw
        REQUIRE_NOTHROW(memoryManager_->deallocate(ptr));
    }
    
    SECTION("allocate() with alignment works") {
        // RED: This drives alignment handling
        void* ptr = memoryManager_->allocate(1024, 64);
        REQUIRE(ptr != nullptr);
        
        // Check alignment
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        REQUIRE((address % 64) == 0);
        
        memoryManager_->deallocate(ptr);
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Aligned Memory Operations", "[memory][aligned]") {
    SetUp();
    
    SECTION("allocateAligned() returns aligned memory") {
        // RED: This drives aligned allocation
        void* ptr = memoryManager_->allocateAligned(1024, 128);
        REQUIRE(ptr != nullptr);
        
        // Verify alignment
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        REQUIRE((address % 128) == 0);
        
        // Memory should be usable
        std::memset(ptr, 0xAA, 1024);
        REQUIRE(static_cast<char*>(ptr)[0] == static_cast<char>(0xAA));
        
        memoryManager_->deallocateAligned(ptr);
    }
    
    SECTION("allocateAligned() with different alignments") {
        // RED: This drives various alignment support
        std::vector<void*> allocations;
        std::vector<size_t> alignments = {8, 16, 32, 64, 128, 256};
        
        for (size_t alignment : alignments) {
            void* ptr = memoryManager_->allocateAligned(512, alignment);
            REQUIRE(ptr != nullptr);
            
            uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
            REQUIRE((address % alignment) == 0);
            
            allocations.push_back(ptr);
        }
        
        // Clean up
        for (void* ptr : allocations) {
            memoryManager_->deallocateAligned(ptr);
        }
    }
    
    SECTION("deallocateAligned() handles valid pointers") {
        // RED: This drives aligned deallocation
        void* ptr = memoryManager_->allocateAligned(256, 32);
        REQUIRE(ptr != nullptr);
        
        REQUIRE_NOTHROW(memoryManager_->deallocateAligned(ptr));
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Memory Reallocation", "[memory][reallocation]") {
    SetUp();
    
    SECTION("reallocate() grows memory blocks") {
        // RED: This drives reallocation implementation
        void* ptr = memoryManager_->allocate(128);
        REQUIRE(ptr != nullptr);
        
        // Fill with test pattern
        std::memset(ptr, 0x33, 128);
        
        // Reallocate to larger size
        void* newPtr = memoryManager_->reallocate(ptr, 256);
        REQUIRE(newPtr != nullptr);
        
        // Original data should be preserved
        REQUIRE(static_cast<char*>(newPtr)[0] == 0x33);
        REQUIRE(static_cast<char*>(newPtr)[127] == 0x33);
        
        memoryManager_->deallocate(newPtr);
    }
    
    SECTION("reallocate() shrinks memory blocks") {
        // RED: This drives shrinking reallocation
        void* ptr = memoryManager_->allocate(512);
        REQUIRE(ptr != nullptr);
        
        std::memset(ptr, 0x77, 512);
        
        void* newPtr = memoryManager_->reallocate(ptr, 128);
        REQUIRE(newPtr != nullptr);
        
        // First 128 bytes should be preserved
        REQUIRE(static_cast<char*>(newPtr)[0] == 0x77);
        REQUIRE(static_cast<char*>(newPtr)[127] == 0x77);
        
        memoryManager_->deallocate(newPtr);
    }
    
    SECTION("reallocate() handles nullptr input") {
        // RED: This drives nullptr reallocation (equivalent to allocate)
        void* ptr = memoryManager_->reallocate(nullptr, 256);
        REQUIRE(ptr != nullptr);
        
        memoryManager_->deallocate(ptr);
    }
    
    SECTION("reallocate() handles zero size") {
        // RED: This drives zero-size reallocation (equivalent to deallocate)
        void* ptr = memoryManager_->allocate(128);
        REQUIRE(ptr != nullptr);
        
        void* result = memoryManager_->reallocate(ptr, 0);
        // Result behavior can vary - either nullptr or valid pointer
        // The important thing is it doesn't crash
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Memory Tracking and Statistics", "[memory][tracking]") {
    SetUp();
    
    SECTION("getAllocatedSize() returns correct size") {
        // RED: This drives size tracking
        void* ptr = memoryManager_->allocate(1024);
        REQUIRE(ptr != nullptr);
        
        size_t allocatedSize = memoryManager_->getAllocatedSize(ptr);
        REQUIRE(allocatedSize >= 1024); // May be larger due to overhead/alignment
        
        memoryManager_->deallocate(ptr);
    }
    
    SECTION("getTotalAllocated() tracks memory usage") {
        // RED: This drives total allocation tracking
        size_t initialTotal = memoryManager_->getTotalAllocated();
        
        void* ptr1 = memoryManager_->allocate(256);
        void* ptr2 = memoryManager_->allocate(512);
        
        size_t afterAllocation = memoryManager_->getTotalAllocated();
        REQUIRE(afterAllocation > initialTotal);
        
        memoryManager_->deallocate(ptr1);
        memoryManager_->deallocate(ptr2);
        
        size_t afterDeallocation = memoryManager_->getTotalAllocated();
        REQUIRE(afterDeallocation <= afterAllocation);
    }
    
    SECTION("getPeakAllocated() tracks peak usage") {
        // RED: This drives peak tracking
        size_t initialPeak = memoryManager_->getPeakAllocated();
        
        // Allocate significant memory
        std::vector<void*> allocations;
        for (int i = 0; i < 10; ++i) {
            allocations.push_back(memoryManager_->allocate(1024));
        }
        
        size_t peakAfterAllocation = memoryManager_->getPeakAllocated();
        REQUIRE(peakAfterAllocation > initialPeak);
        
        // Deallocate some memory
        for (int i = 0; i < 5; ++i) {
            memoryManager_->deallocate(allocations[i]);
        }
        
        // Peak should remain high
        size_t peakAfterSomeDeallocation = memoryManager_->getPeakAllocated();
        REQUIRE(peakAfterSomeDeallocation == peakAfterAllocation);
        
        // Clean up remaining
        for (int i = 5; i < 10; ++i) {
            memoryManager_->deallocate(allocations[i]);
        }
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Memory Validation and Debugging", "[memory][validation]") {
    SetUp();
    
    SECTION("validateHeap() works without errors") {
        // RED: This drives heap validation
        bool isValid = memoryManager_->validateHeap();
        REQUIRE(isValid == true);
        
        // Should still be valid after allocations
        void* ptr = memoryManager_->allocate(128);
        REQUIRE(memoryManager_->validateHeap() == true);
        
        memoryManager_->deallocate(ptr);
        REQUIRE(memoryManager_->validateHeap() == true);
    }
    
    SECTION("reportLeaks() completes without errors") {
        // RED: This drives leak reporting
        REQUIRE_NOTHROW(memoryManager_->reportLeaks());
        
        // Should work even with active allocations
        void* ptr = memoryManager_->allocate(64);
        REQUIRE_NOTHROW(memoryManager_->reportLeaks());
        
        memoryManager_->deallocate(ptr);
        REQUIRE_NOTHROW(memoryManager_->reportLeaks());
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Error Handling and Edge Cases", "[memory][error_handling]") {
    SetUp();
    
    SECTION("Large allocation requests") {
        // RED: This drives large allocation handling
        // Try to allocate a reasonable large block (not too large to fail)
        void* ptr = memoryManager_->allocate(1024 * 1024); // 1MB
        if (ptr != nullptr) {
            // If allocation succeeds, test it
            static_cast<char*>(ptr)[0] = 42;
            REQUIRE(static_cast<char*>(ptr)[0] == 42);
            memoryManager_->deallocate(ptr);
        }
        // If allocation fails, that's also acceptable behavior
    }
    
    SECTION("Zero-size allocation") {
        // RED: This drives zero-size handling
        void* ptr = memoryManager_->allocate(0);
        // Behavior can vary - either nullptr or valid pointer
        if (ptr != nullptr) {
            memoryManager_->deallocate(ptr);
        }
    }
    
    SECTION("Multiple allocations maintain independence") {
        // RED: This drives allocation independence
        void* ptr1 = memoryManager_->allocate(64);
        void* ptr2 = memoryManager_->allocate(64);
        void* ptr3 = memoryManager_->allocate(64);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr3 != nullptr);
        
        // All pointers should be different
        REQUIRE(ptr1 != ptr2);
        REQUIRE(ptr2 != ptr3);
        REQUIRE(ptr1 != ptr3);
        
        // Memory should be independent
        static_cast<char*>(ptr1)[0] = 1;
        static_cast<char*>(ptr2)[0] = 2;
        static_cast<char*>(ptr3)[0] = 3;
        
        REQUIRE(static_cast<char*>(ptr1)[0] == 1);
        REQUIRE(static_cast<char*>(ptr2)[0] == 2);
        REQUIRE(static_cast<char*>(ptr3)[0] == 3);
        
        memoryManager_->deallocate(ptr1);
        memoryManager_->deallocate(ptr2);
        memoryManager_->deallocate(ptr3);
    }
}

TEST_CASE_METHOD(MemoryTestFixture, "Interface Contract Compliance", "[memory][contracts]") {
    SetUp();
    
    SECTION("Memory manager maintains valid state") {
        // RED: This validates interface contracts
        REQUIRE(memoryManager_ != nullptr);
        
        // Should handle repeated operations
        for (int i = 0; i < 5; ++i) {
            void* ptr = memoryManager_->allocate(128);
            REQUIRE(ptr != nullptr);
            
            std::memset(ptr, i, 128);
            
            size_t allocatedSize = memoryManager_->getAllocatedSize(ptr);
            REQUIRE(allocatedSize >= 128);
            
            memoryManager_->deallocate(ptr);
        }
        
        // Validation should still work
        REQUIRE(memoryManager_->validateHeap() == true);
        
        // Statistics should be accessible
        size_t total = memoryManager_->getTotalAllocated();
        size_t peak = memoryManager_->getPeakAllocated();
        REQUIRE(peak >= total); // Peak should be at least current total
    }
}