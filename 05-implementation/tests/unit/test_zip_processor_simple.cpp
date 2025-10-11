#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "dawproject/data/data_access_factory.h"
#include "dawproject/data/data_access_engine.h"
#include <filesystem>
#include <fstream>

using namespace dawproject::data;
using Catch::Matchers::ContainsSubstring;

TEST_CASE("ZIP Processor Simple Coverage", "[zip][simple]") {
    auto zipProcessor = DataAccessFactory::createZIPProcessor();
    REQUIRE(zipProcessor != nullptr);
    
    SECTION("Empty path validation") {
        auto result = zipProcessor->listEntries("");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Archive path cannot be empty"));
    }
    
    SECTION("Nonexistent file") {
        auto result = zipProcessor->listEntries("nonexistent.zip");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Archive file does not exist"));
    }
    
    SECTION("Valid archive validation") {
        // Create a temporary test file
        auto tempDir = std::filesystem::temp_directory_path();
        auto testFile = tempDir / "test_archive.zip";
        
        // Create test file with some content
        std::ofstream file(testFile, std::ios::binary);
        std::vector<uint8_t> testData = {0x50, 0x4B, 0x03, 0x04}; // ZIP signature
        file.write(reinterpret_cast<const char*>(testData.data()), testData.size());
        file.close();
        
        // Test validation
        REQUIRE(zipProcessor->isValidArchive(testFile));
        
        // Test listing entries
        auto listResult = zipProcessor->listEntries(testFile);
        REQUIRE(listResult.success);
        
        const auto& entries = listResult.value;
        REQUIRE(entries.size() == 1);
        REQUIRE(entries[0].name == "project.xml");
        
        // Test extraction
        auto extractResult = zipProcessor->extractEntry(testFile, "project.xml");
        REQUIRE(extractResult.success);
        REQUIRE(extractResult.value == testData);
        
        // Cleanup
        std::filesystem::remove(testFile);
    }
    
    SECTION("Add entry test") {
        auto tempDir = std::filesystem::temp_directory_path();
        auto testFile = tempDir / "new_archive.zip";
        
        std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
        auto addResult = zipProcessor->addEntry(testFile, "project.xml", testData);
        REQUIRE(addResult.success);
        REQUIRE(addResult.value == testData.size());
        
        // Verify file was created
        REQUIRE(std::filesystem::exists(testFile));
        
        // Cleanup
        std::filesystem::remove(testFile);
    }
}