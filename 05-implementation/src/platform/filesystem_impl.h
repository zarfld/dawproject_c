/**
 * @file filesystem_impl.h
 * @brief Minimal FileSystem Implementation for TDD GREEN Phase
 * 
 * Implements IFileSystem interface with minimal functionality to make tests pass.
 * Following XP Simple Design: write only enough code to pass the tests.
 */

#pragma once

#include "dawproject/platform/interfaces.h"
#include "dawproject/platform/factory.h"
#include <filesystem>
#include <memory>
#include <fstream>

namespace dawproject::platform {

/**
 * @brief Minimal file system implementation for TDD GREEN phase
 * 
 * This is the simplest possible implementation that satisfies the interface contract.
 * It will be refactored and improved in subsequent TDD cycles.
 */
class FileSystemImpl : public IFileSystem {
public:
    FileSystemImpl() = default;
    virtual ~FileSystemImpl() = default;

    // Essential file operations
    bool exists(const std::filesystem::path& path) const override {
        return std::filesystem::exists(path);
    }

    bool isFile(const std::filesystem::path& path) const override {
        return std::filesystem::is_regular_file(path);
    }

    bool isDirectory(const std::filesystem::path& path) const override {
        return std::filesystem::is_directory(path);
    }

    // Directory operations - minimal implementation
    bool createDirectory(const std::filesystem::path& path, 
                        bool recursive = true) override {
        (void)recursive; // Suppress unused parameter warning
        return std::filesystem::create_directories(path);
    }

    bool removeFile(const std::filesystem::path& path) override {
        return std::filesystem::remove(path);
    }

    // File metadata - minimal implementation
    FileAttributes getFileAttributes(const std::filesystem::path& path) const override {
        try {
            FileAttributes attrs;
            attrs.size = std::filesystem::file_size(path);
            auto ftime = std::filesystem::last_write_time(path);
            attrs.modified = std::chrono::system_clock::from_time_t(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
            attrs.created = attrs.modified; // Simplified
            attrs.accessed = attrs.modified; // Simplified
            attrs.isReadOnly = false; // Simplified
            attrs.isHidden = false;   // Simplified
            attrs.isDirectory = std::filesystem::is_directory(path);
            return attrs;
        } catch (const std::filesystem::filesystem_error& e) {
            throw PlatformException("Failed to get file attributes: " + std::string(e.what()));
        }
    }

    uint64_t getFileSize(const std::filesystem::path& path) const override {
        try {
            return std::filesystem::file_size(path);
        } catch (const std::filesystem::filesystem_error& e) {
            throw PlatformException("Failed to get file size: " + std::string(e.what()));
        }
    }

    std::chrono::system_clock::time_point getLastModified(
        const std::filesystem::path& path) const override {
        try {
            auto ftime = std::filesystem::last_write_time(path);
            return std::chrono::system_clock::from_time_t(
                std::chrono::duration_cast<std::chrono::seconds>(
                    ftime.time_since_epoch()).count());
        } catch (const std::filesystem::filesystem_error& e) {
            throw PlatformException("Failed to get last modified time: " + std::string(e.what()));
        }
    }

    // Path operations - minimal implementation
    std::filesystem::path getTempDirectory() const override {
        return std::filesystem::temp_directory_path();
    }

    std::filesystem::path makeAbsolute(const std::filesystem::path& path) const override {
        return std::filesystem::absolute(path);
    }

    std::filesystem::path makeRelative(const std::filesystem::path& path,
                                      const std::filesystem::path& base) const override {
        return std::filesystem::relative(path, base);
    }

    // File I/O operations - minimal implementation
    std::unique_ptr<IFileInputStream> openForReading(
        const std::filesystem::path& path) override;
    
    std::unique_ptr<IFileOutputStream> openForWriting(
        const std::filesystem::path& path, bool overwrite = true) override;
};

/**
 * @brief Minimal file input stream implementation
 */
class FileInputStreamImpl : public IFileInputStream {
public:
    explicit FileInputStreamImpl(const std::filesystem::path& path);
    virtual ~FileInputStreamImpl() = default;

    size_t read(void* buffer, size_t size) override;
    bool seek(int64_t offset, SeekOrigin origin = SeekOrigin::Begin) override;
    int64_t tell() const override;
    bool isEOF() const override;
    void close() override;

private:
    std::ifstream file_;
};

/**
 * @brief Minimal file output stream implementation  
 */
class FileOutputStreamImpl : public IFileOutputStream {
public:
    explicit FileOutputStreamImpl(const std::filesystem::path& path, bool overwrite);
    virtual ~FileOutputStreamImpl() = default;

    size_t write(const void* buffer, size_t size) override;
    bool seek(int64_t offset, SeekOrigin origin = SeekOrigin::Begin) override;
    int64_t tell() const override;
    void flush() override;
    void close() override;

private:
    std::ofstream file_;
};

} // namespace dawproject::platform