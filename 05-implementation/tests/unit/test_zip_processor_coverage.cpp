#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "dawproject/data/data_access_factory.h"
#include "dawproject/data/data_access_engine.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>

using namespace dawproject::data;
using Catch::Matchers::ContainsSubstring;

namespace {
    // Test fixture for ZIP processor coverage
    class ZIPProcessorFixture {
    public:
        ZIPProcessorFixture() : zipProcessor_(DataAccessFactory::createZIPProcessor()) {
            // Set up test directory
            testDir_ = std::filesystem::temp_directory_path() / "dawproject_zip_test";
            std::filesystem::create_directories(testDir_);
            
            // Create test files
            validFile_ = testDir_ / "valid_archive.zip";
            emptyFile_ = testDir_ / "empty_archive.zip"; 
            nonexistentFile_ = testDir_ / "nonexistent.zip";
            directoryPath_ = testDir_ / "test_directory";
            largeFile_ = testDir_ / "large_archive.zip";
            readOnlyFile_ = testDir_ / "readonly_archive.zip";
            
            // Create directory
            std::filesystem::create_directory(directoryPath_);
        }
        
        ~ZIPProcessorFixture() {
            // Cleanup test files
            try {
                std::filesystem::remove_all(testDir_);
            } catch (...) {
                // Ignore cleanup errors
            }
        }
        
        void createValidFile(const std::vector<uint8_t>& content = {0x50, 0x4B, 0x03, 0x04}) {
            std::ofstream file(validFile_, std::ios::binary);
            file.write(reinterpret_cast<const char*>(content.data()), content.size());
        }
        
        void createEmptyFile() {
            std::ofstream file(emptyFile_, std::ios::binary);
            // File is empty by default
        }
        
        void createLargeFile() {
            std::ofstream file(largeFile_, std::ios::binary);
            // Create file larger than 100MB limit
            const size_t largeSize = 101 * 1024 * 1024; // 101MB
            std::vector<uint8_t> data(1024, 0xFF);
            for (size_t i = 0; i < largeSize / 1024; ++i) {
                file.write(reinterpret_cast<const char*>(data.data()), 1024);
            }
        }
        
    protected:
        std::unique_ptr<IZIPProcessor> zipProcessor_;
        std::filesystem::path testDir_;
        std::filesystem::path validFile_;
        std::filesystem::path emptyFile_;
        std::filesystem::path nonexistentFile_;
        std::filesystem::path directoryPath_;
        std::filesystem::path largeFile_;
        std::filesystem::path readOnlyFile_;
    };
}

TEST_CASE_METHOD(ZIPProcessorFixture, "ZIP Processor Factory Creation", "[zip][factory]") {
    REQUIRE(zipProcessor_ != nullptr);
}

TEST_CASE_METHOD(ZIPProcessorFixture, "listEntries - Input Validation", "[zip][listEntries][validation]") {
    
    SECTION("Empty path") {
        auto result = zipProcessor_->listEntries("");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Archive path cannot be empty"));
    }
    
    SECTION("Nonexistent file") {
        auto result = zipProcessor_->listEntries(nonexistentFile_);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive file does not exist"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(nonexistentFile_.string()));
    }
    
    SECTION("Directory path") {
        auto result = zipProcessor_->listEntries(directoryPath_);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Path is not a regular file"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(directoryPath_.string()));
    }
    
    SECTION("Empty file") {
        createEmptyFile();
        auto result = zipProcessor_->listEntries(emptyFile_);
        REQUIRE(result.isSuccess());
        REQUIRE(result.getValue().empty());
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "listEntries - Success Cases", "[zip][listEntries][success]") {
    
    SECTION("Valid file") {
        createValidFile();
        auto result = zipProcessor_->listEntries(validFile_);
        REQUIRE(result.isSuccess());
        
        const auto& entries = result.getValue();
        REQUIRE(entries.size() == 1);
        REQUIRE(entries[0].name == "project.xml");
        REQUIRE(entries[0].path == "project.xml");
        REQUIRE_FALSE(entries[0].isDirectory);
        REQUIRE(entries[0].compressedSize > 0);
        REQUIRE(entries[0].uncompressedSize > 0);
    }
    
    SECTION("File time handling") {
        createValidFile();
        // Wait a small amount to ensure time difference
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        auto result = zipProcessor_->listEntries(validFile_);
        REQUIRE(result.isSuccess());
        
        const auto& entries = result.getValue();
        REQUIRE(entries.size() == 1);
        // Time should be reasonably recent (within last minute)
        auto now = std::chrono::system_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - entries[0].modified);
        REQUIRE(timeDiff.count() < 60);
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "extractEntry - Input Validation", "[zip][extractEntry][validation]") {
    
    SECTION("Empty archive path") {
        auto result = zipProcessor_->extractEntry("", "project.xml");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive path cannot be empty"));
    }
    
    SECTION("Empty entry name") {
        createValidFile();
        auto result = zipProcessor_->extractEntry(validFile_, "");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Entry name cannot be empty"));
    }
    
    SECTION("Nonexistent archive") {
        auto result = zipProcessor_->extractEntry(nonexistentFile_, "project.xml");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive file does not exist"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(nonexistentFile_.string()));
    }
    
    SECTION("Directory as archive path") {
        auto result = zipProcessor_->extractEntry(directoryPath_, "project.xml");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Path is not a regular file"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(directoryPath_.string()));
    }
    
    SECTION("Empty archive file") {
        createEmptyFile();
        auto result = zipProcessor_->extractEntry(emptyFile_, "project.xml");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive file is empty"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(emptyFile_.string()));
    }
    
    SECTION("File too large") {
        createLargeFile();
        auto result = zipProcessor_->extractEntry(largeFile_, "project.xml");
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive file too large"));
        REQUIRE_THAT(result.getError(), ContainsSubstring(largeFile_.string()));
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "extractEntry - Success Cases", "[zip][extractEntry][success]") {
    
    SECTION("Extract valid content") {
        std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        createValidFile(testData);
        
        auto result = zipProcessor_->extractEntry(validFile_, "project.xml");
        REQUIRE(result.isSuccess());
        
        const auto& data = result.getValue();
        REQUIRE(data == testData);
    }
    
    SECTION("Extract from large valid file") {
        // Create file just under the limit
        std::vector<uint8_t> largeData(50 * 1024 * 1024, 0xAA); // 50MB
        createValidFile(largeData);
        
        auto result = zipProcessor_->extractEntry(validFile_, "project.xml");
        REQUIRE(result.isSuccess());
        
        const auto& data = result.getValue();
        REQUIRE(data.size() == largeData.size());
        REQUIRE(data == largeData);
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "addEntry - Input Validation", "[zip][addEntry][validation]") {
    
    SECTION("Empty archive path") {
        std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        auto result = zipProcessor_->addEntry("", "project.xml", data);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Archive path cannot be empty"));
    }
    
    SECTION("Empty entry name") {
        std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        auto result = zipProcessor_->addEntry(validFile_, "", data);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Entry name cannot be empty"));
    }
    
    SECTION("Path traversal attack - Unix style") {
        std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        auto result = zipProcessor_->addEntry(validFile_, "../../../etc/passwd", data);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Invalid entry name"));
        REQUIRE_THAT(result.getError(), ContainsSubstring("path traversal detected"));
    }
    
    SECTION("Path traversal attack - Windows style") {
        std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
        auto result = zipProcessor_->addEntry(validFile_, "..\\..\\windows\\system32\\config", data);
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE_THAT(result.getError(), ContainsSubstring("Invalid entry name"));
        REQUIRE_THAT(result.getError(), ContainsSubstring("path traversal detected"));
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "addEntry - Success Cases", "[zip][addEntry][success]") {
    
    SECTION("Add entry with data") {
        std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        auto result = zipProcessor_->addEntry(validFile_, "project.xml", testData);
        REQUIRE(result.isSuccess());
        REQUIRE(result.getValue() == testData.size());
        
        // Verify file was created
        REQUIRE(std::filesystem::exists(validFile_));
        
        // Verify file content
        std::ifstream file(validFile_, std::ios::binary);
        std::vector<uint8_t> readData(testData.size());
        file.read(reinterpret_cast<char*>(readData.data()), testData.size());
        REQUIRE(readData == testData);
    }
    
    SECTION("Add empty entry") {
        std::vector<uint8_t> emptyData;
        auto result = zipProcessor_->addEntry(validFile_, "empty.xml", emptyData);
        REQUIRE(result.isSuccess());
        REQUIRE(result.getValue() == 0);
        
        // Verify empty file was created
        REQUIRE(std::filesystem::exists(validFile_));
        REQUIRE(std::filesystem::file_size(validFile_) == 0);
    }
    
    SECTION("Create directory structure") {
        std::filesystem::path nestedPath = testDir_ / "nested" / "dir" / "archive.zip";
        std::vector<uint8_t> testData = {0x54, 0x65, 0x73, 0x74}; // "Test"
        
        auto result = zipProcessor_->addEntry(nestedPath, "project.xml", testData);
        REQUIRE(result.isSuccess());
        REQUIRE(result.getValue() == testData.size());
        
        // Verify nested directories and file were created
        REQUIRE(std::filesystem::exists(nestedPath));
        REQUIRE(std::filesystem::exists(nestedPath.parent_path()));
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "isValidArchive - Validation Cases", "[zip][isValidArchive]") {
    
    SECTION("Empty path") {
        REQUIRE_FALSE(zipProcessor_->isValidArchive(""));
    }
    
    SECTION("Nonexistent file") {
        REQUIRE_FALSE(zipProcessor_->isValidArchive(nonexistentFile_));
    }
    
    SECTION("Directory path") {
        REQUIRE_FALSE(zipProcessor_->isValidArchive(directoryPath_));
    }
    
    SECTION("Empty file") {
        createEmptyFile();
        REQUIRE_FALSE(zipProcessor_->isValidArchive(emptyFile_));
    }
    
    SECTION("Valid file") {
        createValidFile();
        REQUIRE(zipProcessor_->isValidArchive(validFile_));
    }
    
    SECTION("Valid file with different content") {
        std::vector<uint8_t> xmlContent = {
            0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20  // "<?xml "
        };
        createValidFile(xmlContent);
        REQUIRE(zipProcessor_->isValidArchive(validFile_));
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "Comprehensive Integration Test", "[zip][integration]") {
    
    SECTION("Full workflow: create, validate, list, extract") {
        // Step 1: Create archive with data
        std::vector<uint8_t> originalData = {
            0x3C, 0x50, 0x72, 0x6F, 0x6A, 0x65, 0x63, 0x74, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 
            0x3D, 0x22, 0x31, 0x2E, 0x30, 0x22, 0x3E, 0x3C, 0x2F, 0x50, 0x72, 0x6F, 0x6A, 0x65, 0x63, 0x74, 0x3E
        }; // "<Project version=\"1.0\"></Project>"
        
        auto addResult = zipProcessor_->addEntry(validFile_, "project.xml", originalData);
        REQUIRE(addResult.isSuccess());
        REQUIRE(addResult.getValue() == originalData.size());
        
        // Step 2: Validate archive
        REQUIRE(zipProcessor_->isValidArchive(validFile_));
        
        // Step 3: List entries
        auto listResult = zipProcessor_->listEntries(validFile_);
        REQUIRE(listResult.isSuccess());
        
        const auto& entries = listResult.getValue();
        REQUIRE(entries.size() == 1);
        REQUIRE(entries[0].name == "project.xml");
        REQUIRE(entries[0].compressedSize == originalData.size());
        
        // Step 4: Extract and verify content
        auto extractResult = zipProcessor_->extractEntry(validFile_, "project.xml");
        REQUIRE(extractResult.isSuccess());
        
        const auto& extractedData = extractResult.getValue();
        REQUIRE(extractedData == originalData);
    }
}

TEST_CASE_METHOD(ZIPProcessorFixture, "Error Path Coverage", "[zip][error][coverage]") {
    
    SECTION("Filesystem exception handling in listEntries") {
        // Test with path that might cause filesystem errors
        std::filesystem::path invalidPath = "C:\\invalid:\\path:\\with:invalid:colons";
        auto result = zipProcessor_->listEntries(invalidPath);
        REQUIRE_FALSE(result.isSuccess());
        // Should handle filesystem exception gracefully
    }
    
    SECTION("Memory allocation simulation") {
        // Test with reasonably large file to exercise memory allocation paths
        std::vector<uint8_t> mediumData(10 * 1024 * 1024, 0xCC); // 10MB
        createValidFile(mediumData);
        
        auto result = zipProcessor_->extractEntry(validFile_, "project.xml");
        // Should succeed as it's under the 100MB limit
        REQUIRE(result.isSuccess());
        REQUIRE(result.getValue().size() == mediumData.size());
    }
}

// Additional test for comprehensive method coverage
TEST_CASE_METHOD(ZIPProcessorFixture, "Edge Cases and Boundary Conditions", "[zip][edge][boundary]") {
    
    SECTION("Boundary size file (exactly 100MB)") {
        // Create file at exact boundary
        std::ofstream file(validFile_, std::ios::binary);
        const size_t exactLimit = 100 * 1024 * 1024; // Exactly 100MB
        std::vector<uint8_t> chunk(1024, 0xDD);
        
        for (size_t written = 0; written < exactLimit; written += chunk.size()) {
            size_t toWrite = std::min(chunk.size(), exactLimit - written);
            file.write(reinterpret_cast<const char*>(chunk.data()), toWrite);
        }
        file.close();
        
        auto result = zipProcessor_->extractEntry(validFile_, "project.xml");
        REQUIRE(result.isSuccess()); // Should succeed at exactly 100MB
        REQUIRE(result.getValue().size() == exactLimit);
    }
    
    SECTION("Entry name validation - valid complex names") {
        std::vector<uint8_t> testData = {0x74, 0x65, 0x73, 0x74}; // "test"
        
        // These should all be valid
        std::vector<std::string> validNames = {
            "project.xml",
            "folder/project.xml", 
            "deep/nested/structure/file.xml",
            "file-with-dashes.xml",
            "file_with_underscores.xml",
            "file with spaces.xml",
            "123numeric.xml",
            "special!@#$%^&*()+={}[]|;:,.<>?.xml"
        };
        
        for (const auto& validName : validNames) {
            std::filesystem::path testPath = testDir_ / ("test_" + std::to_string(std::hash<std::string>{}(validName)) + ".zip");
            auto result = zipProcessor_->addEntry(testPath, validName, testData);
            REQUIRE(result.isSuccess());
            REQUIRE(std::filesystem::exists(testPath));
        }
    }
}