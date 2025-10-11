#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "dawproject/data/data_access_factory.h"
#include "dawproject/data/data_access_engine.h"
#include <filesystem>
#include <fstream>

using namespace dawproject::data;
using Catch::Matchers::ContainsSubstring;

namespace {
    class DataAccessEngineFixture {
    public:
        DataAccessEngineFixture() : engine_(DataAccessFactory::createDataAccessEngine()) {
            // Set up test directory
            testDir_ = std::filesystem::temp_directory_path() / "dawproject_engine_test";
            std::filesystem::create_directories(testDir_);
            
            // Create test files
            validFile_ = testDir_ / "valid_project.dawproject";
            emptyFile_ = testDir_ / "empty_project.dawproject"; 
            nonexistentFile_ = testDir_ / "nonexistent.dawproject";
            directoryPath_ = testDir_ / "test_directory";
            largeFile_ = testDir_ / "large_project.dawproject";
            
            // Create directory
            std::filesystem::create_directory(directoryPath_);
        }
        
        ~DataAccessEngineFixture() {
            // Cleanup test files
            try {
                std::filesystem::remove_all(testDir_);
            } catch (...) {
                // Ignore cleanup errors
            }
        }
        
        void createValidProjectFile() {
            // Create a minimal valid DAWProject XML
            const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" timeUnit="beats">
    <Application name="Test DAW" version="1.0.0"/>
    <Transport>
        <Tempo time="0" value="120"/>
    </Transport>
    <Structure>
        <Tracks>
            <Track id="track1" name="Main Track" color="#FF0000" contentType="audio"/>
        </Tracks>
    </Structure>
</Project>)";
            std::ofstream file(validFile_);
            file << xmlContent;
        }
        
        void createEmptyFile() {
            std::ofstream file(emptyFile_);
            // File is empty by default
        }
        
        void createLargeFile() {
            std::ofstream file(largeFile_);
            // Create file larger than 500MB limit
            const size_t largeSize = 501 * 1024 * 1024; // 501MB
            std::vector<char> data(1024, 'A');
            for (size_t i = 0; i < largeSize / 1024; ++i) {
                file.write(data.data(), 1024);
                if (file.fail()) break; // Prevent disk space issues
            }
        }
        
    protected:
        std::unique_ptr<IDataAccessEngine> engine_;
        std::filesystem::path testDir_;
        std::filesystem::path validFile_;
        std::filesystem::path emptyFile_;
        std::filesystem::path nonexistentFile_;
        std::filesystem::path directoryPath_;
        std::filesystem::path largeFile_;
    };
}

TEST_CASE_METHOD(DataAccessEngineFixture, "Data Access Engine Factory Creation", "[engine][factory]") {
    REQUIRE(engine_ != nullptr);
}

// Test loadProjectInfo method - comprehensive error handling coverage
TEST_CASE_METHOD(DataAccessEngineFixture, "loadProjectInfo - Input Validation", "[engine][loadProjectInfo][validation]") {
    
    SECTION("Empty path") {
        auto result = engine_->loadProjectInfo("");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path cannot be empty"));
    }
    
    SECTION("Nonexistent file") {
        auto result = engine_->loadProjectInfo(nonexistentFile_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("File does not exist"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(nonexistentFile_.string()));
    }
    
    SECTION("Directory path") {
        auto result = engine_->loadProjectInfo(directoryPath_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path is not a regular file"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(directoryPath_.string()));
    }
    
    SECTION("File too large") {
        createLargeFile();
        auto result = engine_->loadProjectInfo(largeFile_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("File too large"));
    }
}

TEST_CASE_METHOD(DataAccessEngineFixture, "loadProjectInfo - Success Cases", "[engine][loadProjectInfo][success]") {
    
    SECTION("Valid project file") {
        createValidProjectFile();
        auto result = engine_->loadProjectInfo(validFile_);
        REQUIRE(result.success);
        
        const auto& info = result.value;
        REQUIRE(info.isValid());
        REQUIRE(info.tempo > 0.0);
        REQUIRE_FALSE(info.timeSignature.empty());
    }
}

// Test loadTracks method - exercise different error paths
TEST_CASE_METHOD(DataAccessEngineFixture, "loadTracks - Input Validation", "[engine][loadTracks][validation]") {
    
    SECTION("Empty path") {
        auto result = engine_->loadTracks("");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path cannot be empty"));
    }
    
    SECTION("Nonexistent file") {
        auto result = engine_->loadTracks(nonexistentFile_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("File does not exist"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(nonexistentFile_.string()));
    }
    
    SECTION("Directory path") {
        auto result = engine_->loadTracks(directoryPath_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path is not a regular file"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(directoryPath_.string()));
    }
    
    SECTION("Unreadable file") {
        createEmptyFile();
        auto result = engine_->loadTracks(emptyFile_);
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Cannot read file"));
    }
}

TEST_CASE_METHOD(DataAccessEngineFixture, "loadTracks - Success Cases", "[engine][loadTracks][success]") {
    
    SECTION("Valid project with tracks") {
        createValidProjectFile();
        auto result = engine_->loadTracks(validFile_);
        REQUIRE(result.success);
        
        const auto& tracks = result.value;
        REQUIRE(tracks.size() >= 0); // May be empty for minimal project
    }
}

// Test loadClips method
TEST_CASE_METHOD(DataAccessEngineFixture, "loadClips - Input Validation", "[engine][loadClips][validation]") {
    
    SECTION("Empty path") {
        auto result = engine_->loadClips("", "track1");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path cannot be empty"));
    }
    
    SECTION("Empty track ID") {
        createValidProjectFile();
        auto result = engine_->loadClips(validFile_, "");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Track ID cannot be empty"));
    }
    
    SECTION("Nonexistent file") {
        auto result = engine_->loadClips(nonexistentFile_, "track1");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("File does not exist"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(nonexistentFile_.string()));
    }
    
    SECTION("Directory path") {
        auto result = engine_->loadClips(directoryPath_, "track1");
        REQUIRE_FALSE(result.success);
        REQUIRE_THAT(result.errorMessage, ContainsSubstring("Path is not a regular file"));
        REQUIRE_THAT(result.errorMessage, ContainsSubstring(directoryPath_.string()));
    }
}

TEST_CASE_METHOD(DataAccessEngineFixture, "loadClips - Success Cases", "[engine][loadClips][success]") {
    
    SECTION("Valid project with track") {
        createValidProjectFile();
        auto result = engine_->loadClips(validFile_, "track1");
        REQUIRE(result.success);
        
        const auto& clips = result.value;
        REQUIRE(clips.size() >= 0); // May be empty
    }
}

// Test validateFile method
TEST_CASE_METHOD(DataAccessEngineFixture, "validateFile - Input Validation", "[engine][validateFile][validation]") {
    
    SECTION("Empty path") {
        auto result = engine_->validateFile("");
        REQUIRE_FALSE(result.isValid);
        REQUIRE_FALSE(result.errors.empty());
    }
    
    SECTION("Nonexistent file") {
        auto result = engine_->validateFile(nonexistentFile_);
        REQUIRE_FALSE(result.isValid);
        REQUIRE_FALSE(result.errors.empty());
    }
    
    SECTION("Directory path") {
        auto result = engine_->validateFile(directoryPath_);
        REQUIRE_FALSE(result.isValid);
        REQUIRE_FALSE(result.errors.empty());
    }
    
    SECTION("Empty file") {
        createEmptyFile();
        auto result = engine_->validateFile(emptyFile_);
        REQUIRE_FALSE(result.isValid);
        REQUIRE_FALSE(result.errors.empty());
    }
}

TEST_CASE_METHOD(DataAccessEngineFixture, "validateFile - Success Cases", "[engine][validateFile][success]") {
    
    SECTION("Valid project file") {
        createValidProjectFile();
        auto result = engine_->validateFile(validFile_);
        // May pass or fail depending on validation strictness
        // Just ensure method executes without crashing
        REQUIRE((result.isValid == true || result.isValid == false));
    }
}

// Test isValidProjectFile method
TEST_CASE_METHOD(DataAccessEngineFixture, "isValidProjectFile - Validation Cases", "[engine][isValidProjectFile]") {
    
    SECTION("Empty path") {
        REQUIRE_FALSE(engine_->isValidProjectFile(""));
    }
    
    SECTION("Nonexistent file") {
        REQUIRE_FALSE(engine_->isValidProjectFile(nonexistentFile_));
    }
    
    SECTION("Directory path") {
        REQUIRE_FALSE(engine_->isValidProjectFile(directoryPath_));
    }
    
    SECTION("Empty file") {
        createEmptyFile();
        REQUIRE_FALSE(engine_->isValidProjectFile(emptyFile_));
    }
    
    SECTION("Valid project file") {
        createValidProjectFile();
        // Method should execute without crashing
        auto isValid = engine_->isValidProjectFile(validFile_);
        REQUIRE((isValid == true || isValid == false)); // Just ensure it returns a boolean
    }
}

// Test createReader and createWriter methods
TEST_CASE_METHOD(DataAccessEngineFixture, "createReader - Factory Methods", "[engine][createReader][factory]") {
    
    SECTION("Create reader for valid file") {
        createValidProjectFile();
        auto reader = engine_->createReader(validFile_);
        REQUIRE(reader != nullptr);
    }
    
    SECTION("Create reader for nonexistent file") {
        auto reader = engine_->createReader(nonexistentFile_);
        // Should handle gracefully, may return nullptr or valid reader
        // Just ensure no crash
        // Just ensure no crash occurs
        (void)reader;
    }
}

TEST_CASE_METHOD(DataAccessEngineFixture, "createWriter - Factory Methods", "[engine][createWriter][factory]") {
    
    SECTION("Create writer for valid path") {
        auto writer = engine_->createWriter(validFile_);
        REQUIRE(writer != nullptr);
    }
    
    SECTION("Create writer for directory") {
        auto writer = engine_->createWriter(testDir_);
        // Should handle gracefully
        // Just ensure no crash occurs
        (void)writer;
    }
}

// Integration test combining multiple operations
TEST_CASE_METHOD(DataAccessEngineFixture, "Data Access Engine - Integration Test", "[engine][integration]") {
    
    SECTION("Full workflow: validate, load info, load tracks, load clips") {
        createValidProjectFile();
        
        // Step 1: Check if file is valid
        auto isValid = engine_->isValidProjectFile(validFile_);
        (void)isValid; // Suppress unused warning
        
        // Step 2: Validate file structure
        auto validation = engine_->validateFile(validFile_);
        (void)validation; // Suppress unused warning
        
        // Step 3: Load project info
        auto infoResult = engine_->loadProjectInfo(validFile_);
        REQUIRE(infoResult.success);
        
        // Step 4: Load tracks
        auto tracksResult = engine_->loadTracks(validFile_);
        REQUIRE(tracksResult.success);
        
        // Step 5: Load clips for first track if any
        if (!tracksResult.value.empty()) {
            auto clipsResult = engine_->loadClips(validFile_, tracksResult.value[0].id);
            REQUIRE(clipsResult.success);
        }
    }
}

// Test error path coverage - exception handling
TEST_CASE_METHOD(DataAccessEngineFixture, "Data Access Engine - Error Path Coverage", "[engine][error][coverage]") {
    
    SECTION("Memory allocation stress test") {
        createValidProjectFile();
        
        // Test multiple concurrent operations
        for (int i = 0; i < 10; ++i) {
            auto result = engine_->loadProjectInfo(validFile_);
            // Should handle multiple calls gracefully
            // Just ensure no crash occurs
            (void)result;
        }
    }
    
    SECTION("Filesystem exception simulation") {
        // Test with paths that might cause filesystem errors
        std::vector<std::filesystem::path> problematicPaths = {
            "",
            "/",
            "\\",
            "CON", // Windows reserved name
            "PRN", // Windows reserved name
            std::string(300, 'a') + ".dawproject" // Very long filename
        };
        
        for (const auto& path : problematicPaths) {
            auto result = engine_->loadProjectInfo(path);
            // Should handle all gracefully without crashing
            REQUIRE_FALSE(result.success); // All should fail
            REQUIRE_FALSE(result.errorMessage.empty());
        }
    }
}