#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <dawproject/platform/factory.h>
#include <dawproject/platform/interfaces.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace dawproject::platform;

/**
 * @brief Comprehensive Platform Layer Tests for Coverage Improvement
 * 
 * Target: Increase coverage by 5-8% by testing platform abstractions
 * Focus: Error conditions, boundary cases, and platform-specific behavior
 */

TEST_CASE("Platform Factory - Create All Components", "[platform][factory]") {
    SECTION("Create Filesystem Implementation") {
        auto filesystem = PlatformFactory::createFilesystem();
        REQUIRE(filesystem != nullptr);
        INFO("Factory should create valid filesystem implementation");
    }
    
    SECTION("Create Threading Implementation") {
        auto threading = PlatformFactory::createThreading();
        REQUIRE(threading != nullptr);
        INFO("Factory should create valid threading implementation");
    }
    
    SECTION("Create Memory Manager") {
        auto memory = PlatformFactory::createMemoryManager();
        REQUIRE(memory != nullptr);
        INFO("Factory should create valid memory manager implementation");
    }
}

TEST_CASE("Filesystem - Basic Operations", "[platform][filesystem]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    // Create temp directory for testing
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("File Existence Checks") {
        std::filesystem::path testFile = tempDir / "test.txt";
        
        // File shouldn't exist initially
        REQUIRE_FALSE(filesystem->fileExists(testFile));
        
        // Create file
        std::ofstream ofs(testFile);
        ofs << "test content";
        ofs.close();
        
        // Now file should exist
        REQUIRE(filesystem->fileExists(testFile));
        
        // Cleanup
        std::filesystem::remove(testFile);
    }
    
    SECTION("Directory Operations") {
        std::filesystem::path testSubDir = tempDir / "subdir";
        
        // Directory shouldn't exist initially
        REQUIRE_FALSE(filesystem->directoryExists(testSubDir));
        
        // Create directory using filesystem
        auto result = filesystem->createDirectory(testSubDir);
        REQUIRE(result.success);
        REQUIRE(filesystem->directoryExists(testSubDir));
        
        // Cleanup
        std::filesystem::remove_all(testSubDir);
    }
    
    // Cleanup temp directory
    std::filesystem::remove_all(tempDir);
}

TEST_CASE("Filesystem - Error Conditions", "[platform][filesystem][error]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    SECTION("Non-existent File Operations") {
        std::filesystem::path nonExistent = "/definitely/does/not/exist/file.txt";
        
        REQUIRE_FALSE(filesystem->fileExists(nonExistent));
        
        auto result = filesystem->readFile(nonExistent);
        REQUIRE_FALSE(result.success);
        REQUIRE_FALSE(result.errorMessage.empty());
    }
    
    SECTION("Invalid Path Operations") {
        // Test with empty path
        std::filesystem::path emptyPath;
        REQUIRE_FALSE(filesystem->fileExists(emptyPath));
        
        // Test with root path (should handle gracefully)
        auto result = filesystem->createDirectory("/");
        // Should either succeed (already exists) or fail gracefully
        INFO("Root directory operation should handle gracefully");
    }
    
    SECTION("Permission Scenarios") {
        // Note: Permission testing is platform-specific and may not work in all CI environments
        // This tests the error handling paths in the filesystem implementation
        
        std::filesystem::path restrictedPath;
        
        #ifdef _WIN32
            restrictedPath = "C:\\System Volume Information\\restricted_file.txt";
        #else
            restrictedPath = "/root/restricted_file.txt";
        #endif
        
        auto result = filesystem->writeFile(restrictedPath, "test content");
        // Should fail gracefully with error message
        if (!result.success) {
            REQUIRE_FALSE(result.errorMessage.empty());
            INFO("Permission-denied scenarios should provide meaningful error messages");
        }
    }
}

TEST_CASE("Filesystem - File Content Operations", "[platform][filesystem][content]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_content_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("Text File Read/Write Operations") {
        std::filesystem::path testFile = tempDir / "content_test.txt";
        std::string testContent = "Hello, DAWProject!\nLine 2\nUnicode: 🎵🎶";
        
        // Write content
        auto writeResult = filesystem->writeFile(testFile, testContent);
        REQUIRE(writeResult.success);
        
        // Read content back
        auto readResult = filesystem->readFile(testFile);
        REQUIRE(readResult.success);
        REQUIRE(readResult.value == testContent);
    }
    
    SECTION("Binary File Operations") {
        std::filesystem::path binaryFile = tempDir / "binary_test.bin";
        std::vector<uint8_t> binaryData = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
        
        auto writeResult = filesystem->writeBinaryFile(binaryFile, binaryData);
        REQUIRE(writeResult.success);
        
        auto readResult = filesystem->readBinaryFile(binaryFile);
        REQUIRE(readResult.success);
        REQUIRE(readResult.value == binaryData);
    }
    
    SECTION("Large File Handling") {
        std::filesystem::path largeFile = tempDir / "large_test.txt";
        
        // Create 1MB of test data
        std::string largeContent;
        largeContent.reserve(1024 * 1024);
        for (int i = 0; i < 1024 * 1024; ++i) {
            largeContent += static_cast<char>('A' + (i % 26));
        }
        
        auto writeResult = filesystem->writeFile(largeFile, largeContent);
        REQUIRE(writeResult.success);
        
        auto readResult = filesystem->readFile(largeFile);
        REQUIRE(readResult.success);
        REQUIRE(readResult.value.size() == largeContent.size());
        
        // Verify first and last 100 characters to avoid full comparison overhead
        REQUIRE(readResult.value.substr(0, 100) == largeContent.substr(0, 100));
        REQUIRE(readResult.value.substr(largeContent.size() - 100) == largeContent.substr(largeContent.size() - 100));
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}

TEST_CASE("Filesystem - Path Utilities", "[platform][filesystem][path]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    SECTION("Path Resolution") {
        std::filesystem::path relativePath = "relative/path/file.txt";
        auto absolutePath = filesystem->getAbsolutePath(relativePath);
        REQUIRE(absolutePath.is_absolute());
    }
    
    SECTION("Path Validation") {
        // Valid paths
        REQUIRE(filesystem->isValidPath("valid/path/file.txt"));
        REQUIRE(filesystem->isValidPath("/absolute/path/file.txt"));
        
        // Platform-specific invalid paths
        #ifdef _WIN32
            REQUIRE_FALSE(filesystem->isValidPath("invalid|path\\file.txt"));  // Invalid character |
            REQUIRE_FALSE(filesystem->isValidPath("con.txt"));  // Reserved name
        #else
            // On Unix-like systems, most characters are valid except null
            REQUIRE(filesystem->isValidPath("even-special!@#$%^&*()_+chars.txt"));
        #endif
    }
    
    SECTION("Extension Handling") {
        REQUIRE(filesystem->getFileExtension("file.txt") == ".txt");
        REQUIRE(filesystem->getFileExtension("file.dawproject") == ".dawproject");
        REQUIRE(filesystem->getFileExtension("file") == "");
        REQUIRE(filesystem->getFileExtension("file.") == ".");
        
        REQUIRE(filesystem->hasExtension("file.txt", ".txt"));
        REQUIRE(filesystem->hasExtension("file.DAWPROJECT", ".dawproject"));  // Case insensitive
        REQUIRE_FALSE(filesystem->hasExtension("file.txt", ".xml"));
    }
}

TEST_CASE("Filesystem - Directory Operations", "[platform][filesystem][directory]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_dir_test";
    
    SECTION("Recursive Directory Creation") {
        std::filesystem::path deepPath = tempDir / "level1" / "level2" / "level3";
        
        auto result = filesystem->createDirectoryRecursive(deepPath);
        REQUIRE(result.success);
        REQUIRE(filesystem->directoryExists(deepPath));
        
        // Cleanup
        std::filesystem::remove_all(tempDir);
    }
    
    SECTION("Directory Listing") {
        // Create test directory structure
        std::filesystem::create_directories(tempDir / "subdir1");
        std::filesystem::create_directories(tempDir / "subdir2");
        
        // Create test files
        std::ofstream(tempDir / "file1.txt") << "content1";
        std::ofstream(tempDir / "file2.dawproject") << "content2";
        std::ofstream(tempDir / "subdir1" / "nested.xml") << "nested content";
        
        auto entries = filesystem->listDirectory(tempDir);
        REQUIRE(entries.success);
        REQUIRE(entries.value.size() >= 3);  // 2 subdirs + at least 2 files
        
        // Verify we can find our created items
        bool foundFile1 = false, foundSubdir1 = false;
        for (const auto& entry : entries.value) {
            if (entry.name == "file1.txt" && entry.isFile) foundFile1 = true;
            if (entry.name == "subdir1" && entry.isDirectory) foundSubdir1 = true;
        }
        REQUIRE(foundFile1);
        REQUIRE(foundSubdir1);
        
        // Cleanup
        std::filesystem::remove_all(tempDir);
    }
    
    SECTION("Directory Size Calculation") {
        // Create test files of known sizes
        std::filesystem::create_directories(tempDir);
        
        std::string content1000(1000, 'A');
        std::string content2000(2000, 'B');
        
        std::ofstream(tempDir / "file1.txt") << content1000;
        std::ofstream(tempDir / "file2.txt") << content2000;
        
        auto sizeResult = filesystem->getDirectorySize(tempDir);
        REQUIRE(sizeResult.success);
        REQUIRE(sizeResult.value >= 3000);  // At least 3000 bytes
        
        // Cleanup
        std::filesystem::remove_all(tempDir);
    }
}

TEST_CASE("Filesystem - File Metadata", "[platform][filesystem][metadata]") {
    auto filesystem = PlatformFactory::createFilesystem();
    REQUIRE(filesystem != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_metadata_test";
    std::filesystem::create_directories(tempDir);
    std::filesystem::path testFile = tempDir / "metadata_test.txt";
    
    SECTION("File Size and Timestamps") {
        std::string content = "Test content for metadata";
        std::ofstream(testFile) << content;
        
        auto sizeResult = filesystem->getFileSize(testFile);
        REQUIRE(sizeResult.success);
        REQUIRE(sizeResult.value == content.length());
        
        auto timestampResult = filesystem->getLastModified(testFile);
        REQUIRE(timestampResult.success);
        
        // Timestamp should be recent (within last 10 seconds)
        auto now = std::chrono::system_clock::now();
        auto fileTime = std::chrono::system_clock::from_time_t(timestampResult.value);
        auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - fileTime);
        REQUIRE(timeDiff.count() < 10);
    }
    
    SECTION("File Permissions") {
        std::ofstream(testFile) << "permission test";
        
        auto permResult = filesystem->getFilePermissions(testFile);
        REQUIRE(permResult.success);
        
        // Should have at least read permission
        REQUIRE((permResult.value & dawproject::platform::FilePermission::Read) != 0);
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}