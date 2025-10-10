#pragma once

#include <string>
#include <filesystem>
#include <stdexcept>

/**
 * @brief Test Data Helper for Standards-Compliant Test Organization
 * 
 * Following IEEE 1012-2016 V&V requirements and XP practices.
 * Provides structured access to test data organized by test type and validity.
 */
namespace TestData {
    
    /**
     * @brief Get the root test data directory path
     * @return Filesystem path to test data root
     */
    inline std::filesystem::path getTestDataPath() {
        // Tests run from build directory, data copied to tests/data/
        return std::filesystem::current_path() / "tests" / "data";
    }
    
    /**
     * @brief Get fixture file path by category and filename
     * @param category Fixture category ("valid", "invalid", "edge-cases")
     * @param filename Name of the test data file
     * @return Full path to the fixture file
     */
    inline std::filesystem::path getFixture(const std::string& category, 
                                           const std::string& filename) {
        return getTestDataPath() / "fixtures" / category / filename;
    }
    
    /**
     * @brief Get valid DAWProject file for testing REQ-F-001, REQ-F-002
     * @param name Optional specific filename (default: "simple-project.dawproject")
     *             Available files: "simple-project.dawproject", "multi-track.dawproject", "empty-project.dawproject"
     * @return Path to valid DAWProject file
     */
    inline std::filesystem::path getValidDAWProject(
        const std::string& name = "simple-project.dawproject") {
        return getFixture("valid", name);
    }
    
    /**
     * @brief Get invalid DAWProject file for error testing
     * @param name Optional specific filename (default: "malformed.dawproject")
     *             Available files: "malformed.dawproject", "missing-project-xml.zip", "not-a-zip.dawproject"
     * @return Path to invalid DAWProject file
     */
    inline std::filesystem::path getInvalidDAWProject(
        const std::string& name = "malformed.dawproject") {
        return getFixture("invalid", name);
    }
    
    /**
     * @brief Get edge case DAWProject file for boundary testing
     * @param name Optional specific filename (default: "empty-project.dawproject")
     *             Note: empty-project.dawproject is located in valid folder
     * @return Path to edge case DAWProject file
     */
    inline std::filesystem::path getEdgeCaseDAWProject(
        const std::string& name = "empty-project.dawproject") {
        // Note: empty-project.dawproject is actually in the valid folder
        if (name == "empty-project.dawproject") {
            return getFixture("valid", name);
        }
        return getFixture("edge-cases", name);
    }
    
    /**
     * @brief Get test file that doesn't exist (for negative testing)
     * @param name Optional specific filename (default: "nonexistent.dawproject")
     * @return Path to non-existent file (guaranteed not to exist)
     */
    inline std::filesystem::path getNonExistentDAWProject(
        const std::string& name = "nonexistent.dawproject") {
        return getTestDataPath() / "nonexistent" / name;
    }
    
    /**
     * @brief Check if test data is properly initialized
     * @return true if test data structure exists, false otherwise
     */
    inline bool isTestDataAvailable() {
        auto testDataRoot = getTestDataPath();
        return std::filesystem::exists(testDataRoot / "fixtures" / "valid") &&
               std::filesystem::exists(testDataRoot / "fixtures" / "invalid");
    }
    
    /**
     * @brief Validate test data availability and throw if missing
     * @throws std::runtime_error if test data is not properly set up
     */
    inline void validateTestDataAvailable() {
        if (!isTestDataAvailable()) {
            throw std::runtime_error(
                "Test data not available. Expected structure:\n"
                "  tests/data/fixtures/valid/\n"
                "  tests/data/fixtures/invalid/\n"
                "Run: cmake --build . --target generate_test_data"
            );
        }
    }
}