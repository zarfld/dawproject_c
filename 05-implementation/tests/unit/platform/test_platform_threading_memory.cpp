#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "platform/threading_impl.h"
#include "platform/memory_impl.h" 
#include "platform/platform_factory.h"
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

using namespace dawproject::platform;

/**
 * @brief Threading and Memory Management Tests for Coverage Improvement
 * 
 * Target: Test platform-specific threading and memory management functionality
 * Focus: Synchronization, resource management, and concurrent access patterns
 */

TEST_CASE("Threading - Basic Synchronization", "[platform][threading]") {
    auto threading = PlatformFactory::createThreading();
    REQUIRE(threading != nullptr);
    
    SECTION("Mutex Operations") {
        auto mutex = threading->createMutex();
        REQUIRE(mutex != nullptr);
        
        // Test lock/unlock
        auto lockResult = mutex->lock();
        REQUIRE(lockResult.success);
        
        auto unlockResult = mutex->unlock();
        REQUIRE(unlockResult.success);
    }
    
    SECTION("Read-Write Lock Operations") {
        auto rwLock = threading->createReadWriteLock();
        REQUIRE(rwLock != nullptr);
        
        // Test read lock
        auto readLockResult = rwLock->lockRead();
        REQUIRE(readLockResult.success);
        
        auto readUnlockResult = rwLock->unlockRead();
        REQUIRE(readUnlockResult.success);
        
        // Test write lock
        auto writeLockResult = rwLock->lockWrite();
        REQUIRE(writeLockResult.success);
        
        auto writeUnlockResult = rwLock->unlockWrite();
        REQUIRE(writeUnlockResult.success);
    }
    
    SECTION("Semaphore Operations") {
        const int initialCount = 3;
        auto semaphore = threading->createSemaphore(initialCount);
        REQUIRE(semaphore != nullptr);
        
        // Should be able to acquire up to initial count
        for (int i = 0; i < initialCount; ++i) {
            auto acquireResult = semaphore->acquire();
            REQUIRE(acquireResult.success);
        }
        
        // Release one
        auto releaseResult = semaphore->release();
        REQUIRE(releaseResult.success);
        
        // Should be able to acquire one more
        auto acquireAfterRelease = semaphore->acquire();
        REQUIRE(acquireAfterRelease.success);
    }
}

TEST_CASE("Threading - Concurrent Access Patterns", "[platform][threading][concurrent]") {
    auto threading = PlatformFactory::createThreading();
    REQUIRE(threading != nullptr);
    
    SECTION("Mutex Protects Shared Resource") {
        auto mutex = threading->createMutex();
        std::atomic<int> sharedCounter{0};
        std::atomic<int> unprotectedCounter{0};
        const int incrementsPerThread = 1000;
        const int numThreads = 4;
        
        std::vector<std::thread> threads;
        
        // Launch threads that increment counters
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&]() {
                for (int i = 0; i < incrementsPerThread; ++i) {
                    // Protected increment
                    mutex->lock();
                    int temp = sharedCounter.load();
                    std::this_thread::sleep_for(std::chrono::microseconds(1)); // Encourage race
                    sharedCounter.store(temp + 1);
                    mutex->unlock();
                    
                    // Unprotected increment (should show race condition)
                    int unprotected = unprotectedCounter.load();
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                    unprotectedCounter.store(unprotected + 1);
                }
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Protected counter should be exact
        REQUIRE(sharedCounter.load() == numThreads * incrementsPerThread);
        
        // Unprotected counter will likely be less due to race conditions
        INFO("Unprotected counter: " + std::to_string(unprotectedCounter.load()) + 
             " vs expected: " + std::to_string(numThreads * incrementsPerThread));
    }
    
    SECTION("Read-Write Lock Allows Concurrent Readers") {
        auto rwLock = threading->createReadWriteLock();
        std::atomic<int> readerCount{0};
        std::atomic<int> maxConcurrentReaders{0};
        const int numReaders = 8;
        
        std::vector<std::thread> readers;
        
        // Launch reader threads
        for (int r = 0; r < numReaders; ++r) {
            readers.emplace_back([&]() {
                rwLock->lockRead();
                
                int current = readerCount.fetch_add(1) + 1;
                
                // Update max concurrent readers
                int expected = maxConcurrentReaders.load();
                while (current > expected && 
                       !maxConcurrentReaders.compare_exchange_weak(expected, current)) {
                    expected = maxConcurrentReaders.load();
                }
                
                // Simulate reading work
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                readerCount.fetch_sub(1);
                rwLock->unlockRead();
            });
        }
        
        // Wait for all readers
        for (auto& reader : readers) {
            reader.join();
        }
        
        // Should have had multiple concurrent readers
        REQUIRE(maxConcurrentReaders.load() > 1);
        INFO("Max concurrent readers: " + std::to_string(maxConcurrentReaders.load()));
    }
}

TEST_CASE("Threading - Thread Pool Operations", "[platform][threading][pool]") {
    auto threading = PlatformFactory::createThreading();
    REQUIRE(threading != nullptr);
    
    SECTION("Thread Pool Task Execution") {
        const int poolSize = 4;
        auto threadPool = threading->createThreadPool(poolSize);
        REQUIRE(threadPool != nullptr);
        
        std::atomic<int> completedTasks{0};
        const int numTasks = 16;
        
        // Submit tasks to pool
        for (int i = 0; i < numTasks; ++i) {
            auto task = [&completedTasks, i]() {
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                completedTasks.fetch_add(1);
            };
            
            auto submitResult = threadPool->submitTask(std::move(task));
            REQUIRE(submitResult.success);
        }
        
        // Wait for completion with timeout
        auto start = std::chrono::steady_clock::now();
        const auto timeout = std::chrono::seconds(5);
        
        while (completedTasks.load() < numTasks) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            if (std::chrono::steady_clock::now() - start > timeout) {
                FAIL("Thread pool task execution timed out");
            }
        }
        
        REQUIRE(completedTasks.load() == numTasks);
    }
    
    SECTION("Thread Pool Resource Management") {
        auto threadPool = threading->createThreadPool(2);
        REQUIRE(threadPool != nullptr);
        
        auto statusResult = threadPool->getStatus();
        REQUIRE(statusResult.success);
        REQUIRE(statusResult.value.activeThreads <= 2);
        REQUIRE(statusResult.value.queuedTasks == 0);
        
        // Shutdown should work cleanly
        auto shutdownResult = threadPool->shutdown();
        REQUIRE(shutdownResult.success);
        
        // Should not accept new tasks after shutdown
        auto postShutdownTask = []() {};
        auto submitAfterShutdown = threadPool->submitTask(std::move(postShutdownTask));
        REQUIRE_FALSE(submitAfterShutdown.success);
    }
}

TEST_CASE("Memory Management - Basic Operations", "[platform][memory]") {
    auto memoryManager = PlatformFactory::createMemoryManager();
    REQUIRE(memoryManager != nullptr);
    
    SECTION("Memory Allocation and Deallocation") {
        const size_t allocSize = 1024;
        
        auto allocResult = memoryManager->allocate(allocSize);
        REQUIRE(allocResult.success);
        REQUIRE(allocResult.value != nullptr);
        
        // Test memory accessibility (should not crash)
        auto* ptr = static_cast<char*>(allocResult.value);
        ptr[0] = 'T';
        ptr[allocSize - 1] = 'E';
        REQUIRE(ptr[0] == 'T');
        REQUIRE(ptr[allocSize - 1] == 'E');
        
        auto deallocResult = memoryManager->deallocate(allocResult.value);
        REQUIRE(deallocResult.success);
    }
    
    SECTION("Aligned Memory Allocation") {
        const size_t allocSize = 512;
        const size_t alignment = 64;  // 64-byte alignment
        
        auto allocResult = memoryManager->allocateAligned(allocSize, alignment);
        REQUIRE(allocResult.success);
        REQUIRE(allocResult.value != nullptr);
        
        // Check alignment
        auto address = reinterpret_cast<uintptr_t>(allocResult.value);
        REQUIRE((address % alignment) == 0);
        
        auto deallocResult = memoryManager->deallocateAligned(allocResult.value);
        REQUIRE(deallocResult.success);
    }
    
    SECTION("Memory Pool Operations") {
        const size_t blockSize = 256;
        const size_t poolSize = 8;  // 8 blocks
        
        auto poolResult = memoryManager->createPool(blockSize, poolSize);
        REQUIRE(poolResult.success);
        auto& pool = poolResult.value;
        
        // Allocate multiple blocks from pool
        std::vector<void*> blocks;
        for (size_t i = 0; i < poolSize; ++i) {
            auto blockResult = pool->allocate();
            REQUIRE(blockResult.success);
            blocks.push_back(blockResult.value);
        }
        
        // Pool should be exhausted
        auto exhaustedResult = pool->allocate();
        REQUIRE_FALSE(exhaustedResult.success);
        
        // Return one block
        auto returnResult = pool->deallocate(blocks[0]);
        REQUIRE(returnResult.success);
        
        // Should be able to allocate again
        auto retryResult = pool->allocate();
        REQUIRE(retryResult.success);
        
        // Cleanup remaining blocks
        for (size_t i = 1; i < blocks.size(); ++i) {
            pool->deallocate(blocks[i]);
        }
        pool->deallocate(retryResult.value);
    }
}

TEST_CASE("Memory Management - Performance and Limits", "[platform][memory][performance]") {
    auto memoryManager = PlatformFactory::createMemoryManager();
    REQUIRE(memoryManager != nullptr);
    
    SECTION("Large Allocation Handling") {
        // Test large allocation (10MB)
        const size_t largeSize = 10 * 1024 * 1024;
        
        auto largeAllocResult = memoryManager->allocate(largeSize);
        if (largeAllocResult.success) {
            // If allocation succeeds, verify it's usable
            auto* ptr = static_cast<char*>(largeAllocResult.value);
            ptr[0] = 'S';
            ptr[largeSize - 1] = 'E';
            REQUIRE(ptr[0] == 'S');
            REQUIRE(ptr[largeSize - 1] == 'E');
            
            memoryManager->deallocate(largeAllocResult.value);
        } else {
            // If allocation fails, should provide meaningful error
            REQUIRE_FALSE(largeAllocResult.errorMessage.empty());
            INFO("Large allocation failure message: " + largeAllocResult.errorMessage);
        }
    }
    
    SECTION("Memory Usage Tracking") {
        auto usageBefore = memoryManager->getUsageStats();
        REQUIRE(usageBefore.success);
        
        const size_t allocSize = 8192;
        auto allocResult = memoryManager->allocate(allocSize);
        REQUIRE(allocResult.success);
        
        auto usageAfter = memoryManager->getUsageStats();
        REQUIRE(usageAfter.success);
        
        // Usage should have increased
        REQUIRE(usageAfter.value.allocatedBytes >= usageBefore.value.allocatedBytes + allocSize);
        
        memoryManager->deallocate(allocResult.value);
        
        auto usageAfterDealloc = memoryManager->getUsageStats();
        REQUIRE(usageAfterDealloc.success);
        
        // Usage should be back to original (or close)
        // Note: Some allocators may not immediately return memory to system
        INFO("Usage before: " + std::to_string(usageBefore.value.allocatedBytes) +
             ", after alloc: " + std::to_string(usageAfter.value.allocatedBytes) +
             ", after dealloc: " + std::to_string(usageAfterDealloc.value.allocatedBytes));
    }
    
    SECTION("Zero-Size Allocation Handling") {
        auto zeroResult = memoryManager->allocate(0);
        
        // Behavior may vary by implementation
        if (zeroResult.success) {
            // If successful, should be safe to deallocate
            if (zeroResult.value != nullptr) {
                memoryManager->deallocate(zeroResult.value);
            }
        } else {
            // If failed, should have error message
            REQUIRE_FALSE(zeroResult.errorMessage.empty());
        }
    }
}

TEST_CASE("Memory Management - Error Conditions", "[platform][memory][error]") {
    auto memoryManager = PlatformFactory::createMemoryManager();
    REQUIRE(memoryManager != nullptr);
    
    SECTION("Invalid Deallocation") {
        // Try to deallocate null pointer
        auto nullResult = memoryManager->deallocate(nullptr);
        // Should handle gracefully (implementation-defined behavior)
        
        // Try to deallocate invalid pointer (stack address)
        int stackVar = 42;
        auto stackResult = memoryManager->deallocate(&stackVar);
        // Should detect invalid deallocation
        REQUIRE_FALSE(stackResult.success);
    }
    
    SECTION("Double Deallocation Detection") {
        auto allocResult = memoryManager->allocate(1024);
        REQUIRE(allocResult.success);
        
        // First deallocation should succeed
        auto firstDealloc = memoryManager->deallocate(allocResult.value);
        REQUIRE(firstDealloc.success);
        
        // Second deallocation should fail or handle gracefully
        auto secondDealloc = memoryManager->deallocate(allocResult.value);
        // Implementation should detect double-free
        if (!secondDealloc.success) {
            REQUIRE_FALSE(secondDealloc.errorMessage.empty());
        }
    }
    
    SECTION("Extreme Size Allocation") {
        // Try to allocate impossibly large amount
        const size_t extremeSize = std::numeric_limits<size_t>::max();
        
        auto extremeResult = memoryManager->allocate(extremeSize);
        REQUIRE_FALSE(extremeResult.success);
        REQUIRE_FALSE(extremeResult.errorMessage.empty());
    }
}