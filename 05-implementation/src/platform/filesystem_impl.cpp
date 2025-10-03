/**
 * @file filesystem_impl.cpp
 * @brief Minimal FileSystem Implementation for TDD GREEN Phase
 * 
 * Implements the minimal functionality needed to pass our failing tests.
 * Following XP Simple Design principles.
 */

#include "filesystem_impl.h"
#include <stdexcept>
#include "../include/dawproject/platform/factory.h"

namespace dawproject::platform {

// FileSystemImpl implementation
std::unique_ptr<IFileInputStream> FileSystemImpl::openForReading(
    const std::filesystem::path& path) {
    return std::make_unique<FileInputStreamImpl>(path);
}

std::unique_ptr<IFileOutputStream> FileSystemImpl::openForWriting(
    const std::filesystem::path& path, bool overwrite) {
    return std::make_unique<FileOutputStreamImpl>(path, overwrite);
}

// FileInputStreamImpl implementation
FileInputStreamImpl::FileInputStreamImpl(const std::filesystem::path& path) 
    : file_(path, std::ios::binary) {
    if (!file_.is_open()) {
        throw PlatformException("Failed to open file for reading: " + path.string());
    }
}

size_t FileInputStreamImpl::read(void* buffer, size_t size) {
    file_.read(static_cast<char*>(buffer), size);
    return static_cast<size_t>(file_.gcount());
}

bool FileInputStreamImpl::seek(int64_t offset, SeekOrigin origin) {
    std::ios_base::seekdir dir = std::ios_base::beg;
    switch (origin) {
        case SeekOrigin::Begin: dir = std::ios_base::beg; break;
        case SeekOrigin::Current: dir = std::ios_base::cur; break;
        case SeekOrigin::End: dir = std::ios_base::end; break;
        default: dir = std::ios_base::beg; break;
    }
    file_.seekg(offset, dir);
    return !file_.fail();
}

int64_t FileInputStreamImpl::tell() const {
    return static_cast<int64_t>(const_cast<std::ifstream&>(file_).tellg());
}

bool FileInputStreamImpl::isEOF() const {
    return file_.eof();
}

void FileInputStreamImpl::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

// FileOutputStreamImpl implementation
FileOutputStreamImpl::FileOutputStreamImpl(const std::filesystem::path& path, bool overwrite) 
    : file_(path, std::ios::binary | (overwrite ? std::ios::trunc : std::ios::app)) {
    if (!file_.is_open()) {
        throw PlatformException("Failed to open file for writing: " + path.string());
    }
}

size_t FileOutputStreamImpl::write(const void* buffer, size_t size) {
    file_.write(static_cast<const char*>(buffer), size);
    return file_.good() ? size : 0;
}

bool FileOutputStreamImpl::seek(int64_t offset, SeekOrigin origin) {
    std::ios_base::seekdir dir = std::ios_base::beg;
    switch (origin) {
        case SeekOrigin::Begin: dir = std::ios_base::beg; break;
        case SeekOrigin::Current: dir = std::ios_base::cur; break;
        case SeekOrigin::End: dir = std::ios_base::end; break;
        default: dir = std::ios_base::beg; break;
    }
    file_.seekp(offset, dir);
    return !file_.fail();
}

int64_t FileOutputStreamImpl::tell() const {
    return static_cast<int64_t>(const_cast<std::ofstream&>(file_).tellp());
}

void FileOutputStreamImpl::flush() {
    file_.flush();
}

void FileOutputStreamImpl::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

} // namespace dawproject::platform