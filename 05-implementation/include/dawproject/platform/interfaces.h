#pragma once

/**
 * @file interfaces.h
 * @brief Platform Abstraction Layer Interfaces (DES-C-004)
 * 
 * This file defines the platform-independent interfaces for file system operations,
 * threading primitives, and memory management as specified in DES-C-004.
 * 
 * Standard Compliance:
 * - IEEE 1016-2009: Software Design Descriptions
 * - ISO/IEC/IEEE 12207:2017: Implementation Process
 * 
 * Design Principles:
 * - Interface Segregation Principle
 * - Dependency Inversion Principle  
 * - XP Simple Design
 */

#include "types.h"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace dawproject::platform {

    // Forward declarations
    class IFileInputStream;
    class IFileOutputStream;
    class IMutex;
    class ISharedMutex;
    class IConditionVariable;

    /**
     * @brief Seek origin enumeration for file positioning
     */
    enum class SeekOrigin {
        Begin,      ///< Seek from beginning of file
        Current,    ///< Seek from current position
        End         ///< Seek from end of file
    };

    /**
     * @brief Platform detection enumeration
     */
    enum class Platform {
        Windows,    ///< Microsoft Windows
        MacOS,      ///< Apple macOS
        Linux,      ///< Linux distributions
        Unknown     ///< Unrecognized platform
    };

    /**
     * @brief CPU architecture enumeration
     */
    enum class Architecture {
        x86,        ///< 32-bit x86
        x64,        ///< 64-bit x86-64
        ARM,        ///< 32-bit ARM
        ARM64,      ///< 64-bit ARM
        Unknown     ///< Unrecognized architecture
    };

    /**
     * @brief System information structure
     */
    struct SystemInfo {
        Platform platform;              ///< Current platform
        Architecture architecture;      ///< CPU architecture
        std::string osVersion;          ///< OS version string
        size_t totalMemoryMB;          ///< Total system memory in MB
        size_t availableMemoryMB;      ///< Available memory in MB
        int cpuCoreCount;              ///< Number of CPU cores
        bool isLittleEndian;           ///< Byte order
    };

    /**
     * @brief Platform-specific error information
     */
    struct PlatformError {
        int32_t nativeErrorCode;                              ///< OS-specific error code
        std::string message;                                  ///< Error message
        std::string category;                                 ///< Error category
        std::chrono::system_clock::time_point timestamp;     ///< Error timestamp
    };

    /**
     * @brief File information structure
     */
    struct FileAttributes {
        uint64_t size;                                        ///< File size in bytes
        std::chrono::system_clock::time_point created;       ///< Creation time
        std::chrono::system_clock::time_point modified;      ///< Last modification time
        std::chrono::system_clock::time_point accessed;      ///< Last access time
        bool isReadOnly;                                      ///< Read-only flag
        bool isHidden;                                        ///< Hidden file flag
        bool isDirectory;                                     ///< Directory flag
    };

    /**
     * @brief Cross-platform file system interface
     * 
     * Provides platform-independent file system operations following the design
     * specification DES-C-004. All methods are thread-safe unless otherwise noted.
     */
    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;

        /**
         * @brief Open file for reading
         * @param path Path to file to open
         * @return Unique pointer to input stream or nullptr on failure
         * @throws PlatformException on file system errors
         */
        virtual std::unique_ptr<IFileInputStream> openForReading(
            const std::filesystem::path& path) = 0;

        /**
         * @brief Open file for writing
         * @param path Path to file to create/open
         * @param overwrite Whether to overwrite existing file
         * @return Unique pointer to output stream or nullptr on failure
         * @throws PlatformException on file system errors
         */
        virtual std::unique_ptr<IFileOutputStream> openForWriting(
            const std::filesystem::path& path, bool overwrite = true) = 0;

        /**
         * @brief Check if path exists
         * @param path Path to check
         * @return true if path exists, false otherwise
         */
        virtual bool exists(const std::filesystem::path& path) const = 0;

        /**
         * @brief Check if path is a regular file
         * @param path Path to check
         * @return true if path is a file, false otherwise
         */
        virtual bool isFile(const std::filesystem::path& path) const = 0;

        /**
         * @brief Check if path is a directory
         * @param path Path to check
         * @return true if path is a directory, false otherwise
         */
        virtual bool isDirectory(const std::filesystem::path& path) const = 0;

        /**
         * @brief Create directory
         * @param path Directory path to create
         * @param recursive Whether to create parent directories
         * @return true if directory was created or already exists
         * @throws PlatformException on file system errors
         */
        virtual bool createDirectory(const std::filesystem::path& path, 
                                   bool recursive = true) = 0;

        /**
         * @brief Remove file
         * @param path File path to remove
         * @return true if file was removed
         * @throws PlatformException on file system errors
         */
        virtual bool removeFile(const std::filesystem::path& path) = 0;

        /**
         * @brief Get file attributes
         * @param path Path to file
         * @return File attributes structure
         * @throws PlatformException if file doesn't exist
         */
        virtual FileAttributes getFileAttributes(
            const std::filesystem::path& path) const = 0;

        /**
         * @brief Get file size in bytes
         * @param path Path to file
         * @return File size in bytes
         * @throws PlatformException if file doesn't exist
         */
        virtual uint64_t getFileSize(const std::filesystem::path& path) const = 0;

        /**
         * @brief Get last modification time
         * @param path Path to file
         * @return Last modification timestamp
         * @throws PlatformException if file doesn't exist
         */
        virtual std::chrono::system_clock::time_point getLastModified(
            const std::filesystem::path& path) const = 0;

        /**
         * @brief Get system temporary directory
         * @return Path to temporary directory
         */
        virtual std::filesystem::path getTempDirectory() const = 0;

        /**
         * @brief Convert path to absolute path
         * @param path Relative or absolute path
         * @return Absolute path
         */
        virtual std::filesystem::path makeAbsolute(
            const std::filesystem::path& path) const = 0;

        /**
         * @brief Make path relative to base path
         * @param path Path to make relative
         * @param base Base path for relativity
         * @return Relative path
         */
        virtual std::filesystem::path makeRelative(
            const std::filesystem::path& path,
            const std::filesystem::path& base) const = 0;
    };

    /**
     * @brief File input stream interface for reading files
     */
    class IFileInputStream {
    public:
        virtual ~IFileInputStream() = default;

        /**
         * @brief Read data from file
         * @param buffer Buffer to read data into
         * @param size Number of bytes to read
         * @return Number of bytes actually read
         * @throws PlatformException on I/O errors
         */
        virtual size_t read(void* buffer, size_t size) = 0;

        /**
         * @brief Seek to position in file
         * @param offset Offset in bytes
         * @param origin Seek origin (begin, current, end)
         * @return true if seek was successful
         * @throws PlatformException on seek errors
         */
        virtual bool seek(int64_t offset, SeekOrigin origin = SeekOrigin::Begin) = 0;

        /**
         * @brief Get current file position
         * @return Current position in bytes from beginning
         * @throws PlatformException on position query errors
         */
        virtual int64_t tell() const = 0;

        /**
         * @brief Check if at end of file
         * @return true if at end of file
         */
        virtual bool isEOF() const = 0;

        /**
         * @brief Close the file stream
         */
        virtual void close() = 0;
    };

    /**
     * @brief File output stream interface for writing files
     */
    class IFileOutputStream {
    public:
        virtual ~IFileOutputStream() = default;

        /**
         * @brief Write data to file
         * @param buffer Buffer containing data to write
         * @param size Number of bytes to write
         * @return Number of bytes actually written
         * @throws PlatformException on I/O errors
         */
        virtual size_t write(const void* buffer, size_t size) = 0;

        /**
         * @brief Seek to position in file
         * @param offset Offset in bytes
         * @param origin Seek origin (begin, current, end)
         * @return true if seek was successful
         * @throws PlatformException on seek errors
         */
        virtual bool seek(int64_t offset, SeekOrigin origin = SeekOrigin::Begin) = 0;

        /**
         * @brief Get current file position
         * @return Current position in bytes from beginning
         * @throws PlatformException on position query errors
         */
        virtual int64_t tell() const = 0;

        /**
         * @brief Flush any buffered data to file
         * @throws PlatformException on flush errors
         */
        virtual void flush() = 0;

        /**
         * @brief Close the file stream
         */
        virtual void close() = 0;
    };

    /**
     * @brief Threading abstraction interface
     */
    class IThreading {
    public:
        virtual ~IThreading() = default;

        /**
         * @brief Create a mutex for exclusive access
         * @return Unique pointer to mutex
         */
        virtual std::unique_ptr<IMutex> createMutex() = 0;

        /**
         * @brief Create a shared (reader-writer) mutex
         * @return Unique pointer to shared mutex
         */
        virtual std::unique_ptr<ISharedMutex> createSharedMutex() = 0;

        /**
         * @brief Create a condition variable
         * @return Unique pointer to condition variable
         */
        virtual std::unique_ptr<IConditionVariable> createConditionVariable() = 0;

        /**
         * @brief Get current thread identifier
         * @return Thread ID
         */
        virtual uint64_t getCurrentThreadId() const = 0;

        /**
         * @brief Sleep current thread for specified duration
         * @param duration Sleep duration
         */
        virtual void sleep(std::chrono::milliseconds duration) = 0;

        /**
         * @brief Yield current thread's time slice
         */
        virtual void yield() = 0;
    };

    /**
     * @brief Mutex interface for exclusive access synchronization
     */
    class IMutex {
    public:
        virtual ~IMutex() = default;

        /**
         * @brief Lock the mutex (blocking)
         * @throws PlatformException on lock failure
         */
        virtual void lock() = 0;

        /**
         * @brief Try to lock the mutex (non-blocking)
         * @return true if lock acquired, false otherwise
         */
        virtual bool tryLock() = 0;

        /**
         * @brief Unlock the mutex
         * @throws PlatformException on unlock failure
         */
        virtual void unlock() = 0;
    };

    /**
     * @brief Shared mutex interface for reader-writer synchronization
     */
    class ISharedMutex {
    public:
        virtual ~ISharedMutex() = default;

        /**
         * @brief Lock for exclusive (writer) access
         * @throws PlatformException on lock failure
         */
        virtual void lock() = 0;

        /**
         * @brief Try to lock for exclusive access (non-blocking)
         * @return true if lock acquired, false otherwise
         */
        virtual bool tryLock() = 0;

        /**
         * @brief Unlock exclusive access
         * @throws PlatformException on unlock failure
         */
        virtual void unlock() = 0;

        /**
         * @brief Lock for shared (reader) access
         * @throws PlatformException on lock failure
         */
        virtual void lockShared() = 0;

        /**
         * @brief Try to lock for shared access (non-blocking)
         * @return true if lock acquired, false otherwise
         */
        virtual bool tryLockShared() = 0;

        /**
         * @brief Unlock shared access
         * @throws PlatformException on unlock failure
         */
        virtual void unlockShared() = 0;
    };

    /**
     * @brief Condition variable interface for thread coordination
     */
    class IConditionVariable {
    public:
        virtual ~IConditionVariable() = default;

        /**
         * @brief Wait on condition (simplified for TDD)
         * @throws PlatformException on wait failure
         */
        virtual void wait() = 0;

        /**
         * @brief Wait on condition with timeout
         * @param timeout Maximum wait duration
         * @return true if condition was signaled, false if timeout
         * @throws PlatformException on wait failure
         */
        virtual bool waitFor(std::chrono::milliseconds timeout) = 0;

        /**
         * @brief Notify one waiting thread
         */
        virtual void notifyOne() = 0;

        /**
         * @brief Notify all waiting threads
         */
        virtual void notifyAll() = 0;
    };

    /**
     * @brief Memory management interface
     */
    class IMemoryManager {
    public:
        virtual ~IMemoryManager() = default;

        /**
         * @brief Allocate memory block
         * @param size Size in bytes to allocate
         * @param alignment Memory alignment requirement
         * @return Pointer to allocated memory or nullptr on failure
         * @throws PlatformException on allocation failure
         */
        virtual void* allocate(size_t size, size_t alignment = sizeof(void*)) = 0;

        /**
         * @brief Deallocate memory block
         * @param ptr Pointer to memory to deallocate
         */
        virtual void deallocate(void* ptr) = 0;

        /**
         * @brief Reallocate memory block
         * @param ptr Existing memory pointer (can be nullptr)
         * @param newSize New size in bytes
         * @param alignment Memory alignment requirement
         * @return Pointer to reallocated memory or nullptr on failure
         * @throws PlatformException on reallocation failure
         */
        virtual void* reallocate(void* ptr, size_t newSize, 
                               size_t alignment = sizeof(void*)) = 0;

        /**
         * @brief Allocate aligned memory block
         * @param size Size in bytes to allocate
         * @param alignment Memory alignment requirement (power of 2)
         * @return Pointer to aligned memory or nullptr on failure
         * @throws PlatformException on allocation failure
         */
        virtual void* allocateAligned(size_t size, size_t alignment) = 0;

        /**
         * @brief Deallocate aligned memory block
         * @param ptr Pointer to aligned memory to deallocate
         */
        virtual void deallocateAligned(void* ptr) = 0;

        /**
         * @brief Get size of allocated memory block
         * @param ptr Pointer to allocated memory
         * @return Size of allocated block in bytes
         */
        virtual size_t getAllocatedSize(void* ptr) const = 0;

        /**
         * @brief Get total currently allocated memory
         * @return Total allocated memory in bytes
         */
        virtual size_t getTotalAllocated() const = 0;

        /**
         * @brief Get peak allocated memory
         * @return Peak allocated memory in bytes
         */
        virtual size_t getPeakAllocated() const = 0;

        /**
         * @brief Validate heap integrity (debug builds)
         * @return true if heap is valid
         */
        virtual bool validateHeap() const = 0;

        /**
         * @brief Report memory leaks (debug builds)
         */
        virtual void reportLeaks() const = 0;
    };

} // namespace dawproject::platform