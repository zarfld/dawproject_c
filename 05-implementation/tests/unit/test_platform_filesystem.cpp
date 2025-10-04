/**
 * TEST-UNIT-FILESYSTEM-001
 * Trace: REQ-F-001, REQ-NF-P-001, REQ-NF-R-001, REQ-NF-R-002, REQ-NF-M-001, REQ-NF-M-002
 *
 * @file test_platform_filesystem.cpp
 * @brief TDD Unit Tests for Platform File System Interface (DES-C-004)
 * 
 * This file implements Test-Driven Development for the Platform Abstraction Layer
 * file system interface. Following XP practices with Red-Green-Refactor cycle.
 * 
 * Test Strategy:
 * 1. RED: Write failing test for interface requirement
 * 2. GREEN: Write minimal implementation to pass test
 * 3. REFACTOR: Improve design while keeping tests green
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <dawproject/platform/factory.h>
#include <dawproject/platform/interfaces.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace dawproject::platform;

/**
 * @brief Test fixture for file system tests
 * 
 * Provides common setup and teardown for file system testing.
 * Creates temporary files and directories for testing.
 */
class FileSystemTestFixture {
public:
    void setUp() {
        // Create temporary directory for testing
        tempDir_ = std::filesystem::temp_directory_path() / "dawproject_test";
        std::filesystem::create_directories(tempDir_);
        
        // Create factory instance for testing
        factory_ = &PlatformFactory::getInstance();
        
        // Create test files
        testFile_ = tempDir_ / "test_file.txt";
        testContent_ = "Hello, DAW Project C++ Library!\nThis is a test file for TDD.";
        
        // Write test file
        std::ofstream ofs(testFile_);
        ofs << testContent_;
        ofs.close();
    }
    
    void tearDown() {
        // Clean up test files and directories
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
        // Ignore errors during cleanup
    }
    
protected:
    std::filesystem::path tempDir_;
    std::filesystem::path testFile_;
    std::string testContent_;
    PlatformFactory* factory_ = nullptr;
};

/**
 * @brief TDD Test Suite for Platform File System Interface
 * 
 * These tests drive the implementation of the IFileSystem interface
 * following the detailed design specification DES-C-004.
 */
class PlatformFileSystemTests : public FileSystemTestFixture {
public:
    PlatformFileSystemTests() {
        setUp();
    }
    
    ~PlatformFileSystemTests() {
        tearDown();
    }
};

// RED PHASE: First failing test - Factory should create file system
TEST_CASE_METHOD(PlatformFileSystemTests, "FileSystem Factory Creation", "[platform][filesystem][factory]") {
    SECTION("Factory creates valid file system instance") {
        // RED: This should fail initially - no implementation exists yet
        auto fileSystem = factory_->createFileSystem();
        
        // Basic interface contract verification
        REQUIRE(fileSystem != nullptr);
        REQUIRE_NOTHROW(fileSystem.get());
    }
}

// RED PHASE: File existence checking
TEST_CASE_METHOD(PlatformFileSystemTests, "File Existence Operations", "[platform][filesystem][exists]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("exists() returns true for existing file") {
        // RED: This drives the exists() implementation
        bool result = fileSystem->exists(testFile_);
        REQUIRE(result == true);
    }
    
    SECTION("exists() returns false for non-existent file") {
        // RED: Test negative case
        auto nonExistentFile = tempDir_ / "does_not_exist.txt";
        bool result = fileSystem->exists(nonExistentFile);
        REQUIRE(result == false);
    }
    
    SECTION("isFile() returns true for regular file") {
        // RED: This drives the isFile() implementation
        bool result = fileSystem->isFile(testFile_);
        REQUIRE(result == true);
    }
    
    SECTION("isDirectory() returns true for directory") {
        // RED: This drives the isDirectory() implementation
        bool result = fileSystem->isDirectory(tempDir_);
        REQUIRE(result == true);
    }
    
    SECTION("isFile() returns false for directory") {
        // RED: Boundary condition testing
        bool result = fileSystem->isFile(tempDir_);
        REQUIRE(result == false);
    }
}

// RED PHASE: File reading operations
TEST_CASE_METHOD(PlatformFileSystemTests, "File Reading Operations", "[platform][filesystem][read]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("openForReading() returns valid stream for existing file") {
        // RED: This drives the openForReading() implementation
        auto stream = fileSystem->openForReading(testFile_);
        
        REQUIRE(stream != nullptr);
        REQUIRE_FALSE(stream->isEOF());
    }
    
    SECTION("read() returns correct file content") {
        // RED: This drives the read() implementation
        auto stream = fileSystem->openForReading(testFile_);
        REQUIRE(stream != nullptr);
        
        // Read entire file content
        std::vector<char> buffer(testContent_.size() + 1, '\0');
        size_t bytesRead = stream->read(buffer.data(), buffer.size() - 1);
        
        REQUIRE(bytesRead == testContent_.size());
        REQUIRE(std::string(buffer.data()) == testContent_);
    }
    
    SECTION("seek() and tell() work correctly") {
        // RED: This drives seek/tell implementation
        auto stream = fileSystem->openForReading(testFile_);
        REQUIRE(stream != nullptr);
        
        // Test initial position
        REQUIRE(stream->tell() == 0);
        
        // Seek to middle of file
        bool seekResult = stream->seek(5, SeekOrigin::Begin);
        REQUIRE(seekResult == true);
        REQUIRE(stream->tell() == 5);
        
        // Read from middle
        char buffer[10] = {0};
        size_t bytesRead = stream->read(buffer, 5);
        REQUIRE(bytesRead == 5);
        REQUIRE(stream->tell() == 10);
    }
    
    SECTION("openForReading() throws exception for non-existent file") {
        // RED: Error handling test
        auto nonExistentFile = tempDir_ / "does_not_exist.txt";
        
        REQUIRE_THROWS_AS(fileSystem->openForReading(nonExistentFile), 
                         PlatformException);
    }
}

// RED PHASE: File writing operations  
TEST_CASE_METHOD(PlatformFileSystemTests, "File Writing Operations", "[platform][filesystem][write]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("openForWriting() creates valid stream") {
        // RED: This drives the openForWriting() implementation
        auto outputFile = tempDir_ / "output_test.txt";
        auto stream = fileSystem->openForWriting(outputFile);
        
        REQUIRE(stream != nullptr);
        REQUIRE(stream->tell() == 0);
    }
    
    SECTION("write() stores data correctly") {
        // RED: This drives the write() implementation
        auto outputFile = tempDir_ / "output_test.txt";
        auto stream = fileSystem->openForWriting(outputFile);
        REQUIRE(stream != nullptr);
        
        std::string testData = "TDD Test Data\nLine 2";
        size_t bytesWritten = stream->write(testData.c_str(), testData.size());
        
        REQUIRE(bytesWritten == testData.size());
        REQUIRE(stream->tell() == static_cast<int64_t>(testData.size()));
        
        // Flush and close
        stream->flush();
        stream->close();
        
        // Verify file was written correctly
        REQUIRE(fileSystem->exists(outputFile));
        
        // Read back and verify content
        auto readStream = fileSystem->openForReading(outputFile);
        std::vector<char> buffer(testData.size() + 1, '\0');
        size_t bytesRead = readStream->read(buffer.data(), buffer.size() - 1);
        
        REQUIRE(bytesRead == testData.size());
        REQUIRE(std::string(buffer.data()) == testData);
    }
}

// RED PHASE: Directory operations
TEST_CASE_METHOD(PlatformFileSystemTests, "Directory Operations", "[platform][filesystem][directory]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("createDirectory() creates new directory") {
        // RED: This drives the createDirectory() implementation
        auto newDir = tempDir_ / "new_test_directory";
        
        // Should not exist initially
        REQUIRE_FALSE(fileSystem->exists(newDir));
        
        // Create directory
        bool result = fileSystem->createDirectory(newDir);
        REQUIRE(result == true);
        
        // Should exist now
        REQUIRE(fileSystem->exists(newDir));
        REQUIRE(fileSystem->isDirectory(newDir));
    }
    
    SECTION("createDirectory() with recursive flag creates parent directories") {
        // RED: This drives recursive directory creation
        auto deepDir = tempDir_ / "level1" / "level2" / "level3";
        
        bool result = fileSystem->createDirectory(deepDir, true);
        REQUIRE(result == true);
        REQUIRE(fileSystem->exists(deepDir));
        REQUIRE(fileSystem->isDirectory(deepDir));
    }
    
    SECTION("removeFile() deletes existing file") {
        // RED: This drives the removeFile() implementation  
        auto tempFile = tempDir_ / "file_to_delete.txt";
        
        // Create file first
        std::ofstream ofs(tempFile);
        ofs << "temporary content";
        ofs.close();
        
        REQUIRE(fileSystem->exists(tempFile));
        
        // Remove file
        bool result = fileSystem->removeFile(tempFile);
        REQUIRE(result == true);
        REQUIRE_FALSE(fileSystem->exists(tempFile));
    }
}

// RED PHASE: File attributes and metadata
TEST_CASE_METHOD(PlatformFileSystemTests, "File Attributes and Metadata", "[platform][filesystem][metadata]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("getFileSize() returns correct size") {
        // RED: This drives getFileSize() implementation
        uint64_t size = fileSystem->getFileSize(testFile_);
        REQUIRE(size == testContent_.size());
    }
    
    SECTION("getFileAttributes() returns valid attributes") {
        // RED: This drives getFileAttributes() implementation
        auto attributes = fileSystem->getFileAttributes(testFile_);
        
        REQUIRE(attributes.size == testContent_.size());
        REQUIRE_FALSE(attributes.isDirectory);
        REQUIRE(attributes.created != std::chrono::system_clock::time_point{});
        REQUIRE(attributes.modified != std::chrono::system_clock::time_point{});
    }
    
    SECTION("getLastModified() returns valid timestamp") {
        // RED: This drives getLastModified() implementation
        auto timestamp = fileSystem->getLastModified(testFile_);
        REQUIRE(timestamp != std::chrono::system_clock::time_point{});
        
        // Should be recent (within last minute)
        auto now = std::chrono::system_clock::now();
        auto diff = now - timestamp;
        REQUIRE(diff < std::chrono::minutes(1));
    }
}

// RED PHASE: Path operations
TEST_CASE_METHOD(PlatformFileSystemTests, "Path Operations", "[platform][filesystem][paths]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("makeAbsolute() converts relative to absolute path") {
        // RED: This drives makeAbsolute() implementation
        std::filesystem::path relativePath = "relative/path/test.txt";
        auto absolutePath = fileSystem->makeAbsolute(relativePath);
        
        REQUIRE(absolutePath.is_absolute());
        REQUIRE(absolutePath.filename() == "test.txt");
    }
    
    SECTION("getTempDirectory() returns valid temp directory") {
        // RED: This drives getTempDirectory() implementation
        auto tempDir = fileSystem->getTempDirectory();
        
        REQUIRE_FALSE(tempDir.empty());
        REQUIRE(fileSystem->exists(tempDir));
        REQUIRE(fileSystem->isDirectory(tempDir));
    }
    
    SECTION("makeRelative() creates relative path") {
        // RED: This drives makeRelative() implementation
        auto basePath = tempDir_;
        auto targetPath = testFile_;
        
        auto relativePath = fileSystem->makeRelative(targetPath, basePath);
        REQUIRE_FALSE(relativePath.is_absolute());
        REQUIRE(relativePath.filename() == testFile_.filename());
    }
}

// RED PHASE: Error handling and edge cases
TEST_CASE_METHOD(PlatformFileSystemTests, "Error Handling and Edge Cases", "[platform][filesystem][errors]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("Operations on non-existent files throw appropriate exceptions") {
        auto nonExistentFile = tempDir_ / "does_not_exist.txt";
        
        // These should all throw PlatformException
        REQUIRE_THROWS_AS(fileSystem->getFileSize(nonExistentFile), PlatformException);
        REQUIRE_THROWS_AS(fileSystem->getFileAttributes(nonExistentFile), PlatformException);
        REQUIRE_THROWS_AS(fileSystem->getLastModified(nonExistentFile), PlatformException);
        REQUIRE_THROWS_AS(fileSystem->openForReading(nonExistentFile), PlatformException);
    }
    
    SECTION("Stream operations handle invalid states gracefully") {
        auto stream = fileSystem->openForReading(testFile_);
        REQUIRE(stream != nullptr);
        
        // Close stream and verify operations handle closed state
        stream->close();
        
        // These operations on closed stream should handle gracefully
        REQUIRE_NOTHROW(stream->isEOF());
        REQUIRE(stream->tell() == -1); // Indicate invalid state
    }
}

/**
 * @brief TDD Integration Test - Verify interface contract compliance
 * 
 * This test verifies that our implementation adheres to the interface
 * contracts defined in DES-C-004.
 */
TEST_CASE_METHOD(PlatformFileSystemTests, "Interface Contract Compliance", "[platform][filesystem][contract]") {
    auto fileSystem = factory_->createFileSystem();
    REQUIRE(fileSystem != nullptr);
    
    SECTION("All interface methods are callable without crashes") {
        // This test ensures basic interface stability
        REQUIRE_NOTHROW(fileSystem->exists(testFile_));
        REQUIRE_NOTHROW(fileSystem->isFile(testFile_));
        REQUIRE_NOTHROW(fileSystem->isDirectory(tempDir_));
        REQUIRE_NOTHROW(fileSystem->getFileSize(testFile_));
        REQUIRE_NOTHROW(fileSystem->getTempDirectory());
        REQUIRE_NOTHROW(fileSystem->makeAbsolute("test.txt"));
    }
    
    SECTION("Stream RAII behavior works correctly") {
        // Test automatic resource cleanup
        {
            auto stream = fileSystem->openForReading(testFile_);
            REQUIRE(stream != nullptr);
            // Stream should be automatically closed when going out of scope
        }
        
        // No crashes or resource leaks should occur
        REQUIRE(true); // If we get here, RAII worked correctly
    }
}

/**
 * @brief Main test entry point for TDD workflow
 */
int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}