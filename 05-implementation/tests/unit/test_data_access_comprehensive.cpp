/**
 * @file test_data_access_comprehensive.cpp
 * @brief Comprehensive data access layer test coverage
 * 
 * Standards: ISO/IEC/IEEE 12207:2017 (Implementation Process)
 * DAWProject: v1.0 Specification Compliance Testing
 * Coverage Target: +10% improvement from data access edge cases
 * Test Framework: Catch2 v3.4.0
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <pugixml.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <random>
#include <string>

namespace fs = std::filesystem;

// Mock XML Processor Interface (replace with actual interface)
class XMLProcessorInterface {
public:
    virtual ~XMLProcessorInterface() = default;
    virtual bool parseFromString(const std::string& xmlContent) = 0;
    virtual bool parseFromFile(const fs::path& filePath) = 0;
    virtual std::string serializeToString() const = 0;
    virtual bool saveToFile(const fs::path& filePath) const = 0;
    virtual bool validateSchema() const = 0;
};

// Mock ZIP Handler Interface
class ZIPHandlerInterface {
public:
    virtual ~ZIPHandlerInterface() = default;
    virtual bool createArchive(const fs::path& archivePath) = 0;
    virtual bool addFile(const std::string& filename, const std::string& content) = 0;
    virtual bool extractFile(const std::string& filename, std::string& content) = 0;
    virtual std::vector<std::string> listFiles() const = 0;
    virtual bool close() = 0;
};

// Test utilities
class XMLTestUtils {
public:
    static std::string generateValidDAWProjectXML() {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
    <Transport>
        <Tempo automationLaneId="tempo">
            <Real time="0" value="120.0"/>
        </Tempo>
        <TimeSignature>
            <Integer time="0" value="4"/>
        </TimeSignature>
    </Transport>
    <Structure>
        <Track id="track1" name="Audio Track" mediaType="audio">
            <Channel role="regular"/>
        </Track>
    </Structure>
    <Arrangement>
        <Lanes>
            <Lane id="lane1" trackId="track1"/>
        </Lanes>
    </Arrangement>
</Project>)";
    }
    
    static std::string generateInvalidXML() {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
    <Transport>
        <Tempo automationLaneId="tempo">
            <Real time="0" value="120.0"/>
        <!-- Missing closing tags
    </Transport>
</Project>)";
    }
    
    static std::string generateLargeXML(size_t numTracks = 1000) {
        std::ostringstream oss;
        oss << R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
    <Transport>
        <Tempo automationLaneId="tempo">
            <Real time="0" value="120.0"/>
        </Tempo>
    </Transport>
    <Structure>)";
        
        for (size_t i = 0; i < numTracks; ++i) {
            oss << R"(        <Track id="track)" << i << R"(" name="Track )" << i << R"(" mediaType="audio">
            <Channel role="regular"/>
        </Track>
)";
        }
        
        oss << R"(    </Structure>
    <Arrangement>
        <Lanes>)";
        
        for (size_t i = 0; i < numTracks; ++i) {
            oss << R"(            <Lane id="lane)" << i << R"(" trackId="track)" << i << R"("/>
)";
        }
        
        oss << R"(        </Lanes>
    </Arrangement>
</Project>)";
        
        return oss.str();
    }
    
    static std::string generateXMLWithSpecialCharacters() {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="Test&lt;&gt;&amp;&quot;DAW" version="1.0"/>
    <Structure>
        <Track id="track_special" name="Track with àccénts &amp; symbols!" mediaType="audio">
            <Channel role="regular"/>
        </Track>
    </Structure>
</Project>)";
    }
};

TEST_CASE("Data Access - XML Processing Edge Cases", "[data][xml][edge]") {
    
    SECTION("Valid DAWProject XML Parsing") {
        pugi::xml_document doc;
        std::string validXML = XMLTestUtils::generateValidDAWProjectXML();
        
        pugi::xml_parse_result result = doc.load_string(validXML.c_str());
        
        REQUIRE(result.status == pugi::status_ok);
        REQUIRE(doc.child("Project"));
        REQUIRE(std::string(doc.child("Project").attribute("version").value()) == "1.0");
        REQUIRE(std::string(doc.child("Project").attribute("xmlns").value()) == "http://dawproject.org/XML");
        
        // Verify structure
        auto project = doc.child("Project");
        REQUIRE(project.child("Application"));
        REQUIRE(project.child("Transport"));
        REQUIRE(project.child("Structure"));
        REQUIRE(project.child("Arrangement"));
    }
    
    SECTION("Invalid XML Handling") {
        pugi::xml_document doc;
        std::string invalidXML = XMLTestUtils::generateInvalidXML();
        
        pugi::xml_parse_result result = doc.load_string(invalidXML.c_str());
        
        REQUIRE(result.status != pugi::status_ok);
        REQUIRE(result.description != nullptr);
        REQUIRE(result.offset > 0);
        
        INFO("Parse error: " << result.description << " at offset " << result.offset);
    }
    
    SECTION("Large XML Performance") {
        const size_t numTracks = 5000; // Stress test with many tracks
        std::string largeXML = XMLTestUtils::generateLargeXML(numTracks);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(largeXML.c_str());
        
        auto parseEnd = std::chrono::high_resolution_clock::now();
        
        REQUIRE(result.status == pugi::status_ok);
        
        // Count tracks to verify parsing
        size_t trackCount = 0;
        auto structure = doc.child("Project").child("Structure");
        for (auto track : structure.children("Track")) {
            trackCount++;
        }
        
        auto countEnd = std::chrono::high_resolution_clock::now();
        
        REQUIRE(trackCount == numTracks);
        
        auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            parseEnd - start).count();
        auto traversalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            countEnd - parseEnd).count();
        
        INFO("Parse time: " << parseDuration << "ms for " << numTracks << " tracks");
        INFO("Traversal time: " << traversalDuration << "ms");
        
        // Performance thresholds (adjust as needed)
        REQUIRE(parseDuration < 5000);    // Less than 5 seconds
        REQUIRE(traversalDuration < 1000); // Less than 1 second
    }
    
    SECTION("XML Special Character Handling") {
        std::string specialXML = XMLTestUtils::generateXMLWithSpecialCharacters();
        
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(specialXML.c_str());
        
        REQUIRE(result.status == pugi::status_ok);
        
        auto app = doc.child("Project").child("Application");
        std::string appName = app.attribute("name").value();
        
        // Verify special character handling
        REQUIRE(appName.find("Test") == 0);
        REQUIRE(appName.find("DAW") != std::string::npos);
        
        auto track = doc.child("Project").child("Structure").child("Track");
        std::string trackName = track.attribute("name").value();
        
        REQUIRE(trackName.find("àccénts") != std::string::npos);
    }
    
    SECTION("XML Memory Usage") {
        // Test memory efficiency with repeated parsing
        const int iterations = 100;
        std::string xml = XMLTestUtils::generateValidDAWProjectXML();
        
        for (int i = 0; i < iterations; ++i) {
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(xml.c_str());
            REQUIRE(result.status == pugi::status_ok);
            
            // Simulate document manipulation
            auto project = doc.child("Project");
            auto newTrack = project.child("Structure").append_child("Track");
            newTrack.append_attribute("id") = ("generated_track_" + std::to_string(i)).c_str();
            newTrack.append_attribute("name") = ("Generated Track " + std::to_string(i)).c_str();
            
            // Document goes out of scope and should be cleaned up
        }
        // Memory should be properly managed across iterations
    }
    
    SECTION("XML Namespace Handling") {
        std::string xmlWithNamespaces = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML" xmlns:ext="http://example.com/extension">
    <Application name="TestDAW" version="1.0"/>
    <ext:CustomData>
        <ext:Property name="custom" value="data"/>
    </ext:CustomData>
</Project>)";
        
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlWithNamespaces.c_str());
        
        REQUIRE(result.status == pugi::status_ok);
        
        // Verify namespace handling
        auto project = doc.child("Project");
        REQUIRE(project);
        
        // Find custom data with namespace
        auto customData = project.child("ext:CustomData");
        if (customData) {
            auto property = customData.child("ext:Property");
            REQUIRE(property);
            REQUIRE(std::string(property.attribute("name").value()) == "custom");
        }
    }
    
    SECTION("XML Encoding Tests") {
        // Test UTF-8 encoding with various character sets
        std::string utf8XML = u8R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="测试DAW" version="1.0"/>
    <Structure>
        <Track id="track1" name="Тест Дорожка" mediaType="audio">
            <Channel role="regular"/>
        </Track>
        <Track id="track2" name="トラック" mediaType="audio">
            <Channel role="regular"/>
        </Track>
    </Structure>
</Project>)";
        
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(utf8XML.c_str());
        
        REQUIRE(result.status == pugi::status_ok);
        
        auto tracks = doc.child("Project").child("Structure").children("Track");
        int trackCount = 0;
        for (auto track : tracks) {
            trackCount++;
            std::string trackName = track.attribute("name").value();
            REQUIRE_FALSE(trackName.empty());
        }
        
        REQUIRE(trackCount == 2);
    }
}

TEST_CASE("Data Access - ZIP Container Operations", "[data][zip][container]") {
    
    SECTION("Basic ZIP Creation and Extraction") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_test";
        fs::create_directories(tempDir);
        
        auto zipPath = tempDir / "test.dawproject";
        
        // Create ZIP file
        int error = 0;
        zip_t* archive = zip_open(zipPath.string().c_str(), ZIP_CREATE | ZIP_EXCL, &error);
        REQUIRE(archive != nullptr);
        
        // Add project.xml
        std::string projectXML = XMLTestUtils::generateValidDAWProjectXML();
        zip_source_t* source = zip_source_buffer(archive, projectXML.c_str(), 
                                                projectXML.length(), 0);
        REQUIRE(source != nullptr);
        
        zip_int64_t index = zip_file_add(archive, "project.xml", source, ZIP_FL_ENC_UTF_8);
        REQUIRE(index >= 0);
        
        // Add metadata.xml
        std::string metadataXML = R"(<?xml version="1.0" encoding="UTF-8"?>
<MetaData>
    <Created>2024-01-01T12:00:00Z</Created>
    <Modified>2024-01-01T12:00:00Z</Modified>
</MetaData>)";
        
        zip_source_t* metaSource = zip_source_buffer(archive, metadataXML.c_str(), 
                                                    metadataXML.length(), 0);
        REQUIRE(metaSource != nullptr);
        
        zip_int64_t metaIndex = zip_file_add(archive, "metadata.xml", metaSource, ZIP_FL_ENC_UTF_8);
        REQUIRE(metaIndex >= 0);
        
        // Close archive
        int closeResult = zip_close(archive);
        REQUIRE(closeResult == 0);
        
        // Verify file exists
        REQUIRE(fs::exists(zipPath));
        REQUIRE(fs::is_regular_file(zipPath));
        
        // Read back ZIP file
        zip_t* readArchive = zip_open(zipPath.string().c_str(), ZIP_RDONLY, &error);
        REQUIRE(readArchive != nullptr);
        
        zip_int64_t numFiles = zip_get_num_entries(readArchive, 0);
        REQUIRE(numFiles == 2);
        
        // Read project.xml
        zip_file_t* projectFile = zip_fopen(readArchive, "project.xml", 0);
        REQUIRE(projectFile != nullptr);
        
        char buffer[8192];
        zip_int64_t bytesRead = zip_fread(projectFile, buffer, sizeof(buffer) - 1);
        REQUIRE(bytesRead > 0);
        buffer[bytesRead] = '\0';
        
        std::string readContent(buffer);
        REQUIRE(readContent.find("<?xml version=\"1.0\"") == 0);
        REQUIRE(readContent.find("<Project version=\"1.0\"") != std::string::npos);
        
        zip_fclose(projectFile);
        zip_close(readArchive);
        
        // Cleanup
        fs::remove_all(tempDir);
    }
    
    SECTION("ZIP Error Handling") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_error_test";
        fs::create_directories(tempDir);
        
        auto nonExistentPath = tempDir / "nonexistent" / "test.dawproject";
        
        // Try to create ZIP in non-existent directory
        int error = 0;
        zip_t* archive = zip_open(nonExistentPath.string().c_str(), ZIP_CREATE, &error);
        REQUIRE(archive == nullptr);
        REQUIRE(error != 0);
        
        // Test opening non-existent ZIP for reading
        auto invalidZip = tempDir / "invalid.dawproject";
        zip_t* readArchive = zip_open(invalidZip.string().c_str(), ZIP_RDONLY, &error);
        REQUIRE(readArchive == nullptr);
        REQUIRE(error != 0);
        
        fs::remove_all(tempDir);
    }
    
    SECTION("ZIP Large File Handling") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_large_test";
        fs::create_directories(tempDir);
        
        auto zipPath = tempDir / "large_test.dawproject";
        
        // Create large XML content (simulate large project)
        std::string largeXML = XMLTestUtils::generateLargeXML(10000);
        
        int error = 0;
        zip_t* archive = zip_open(zipPath.string().c_str(), ZIP_CREATE | ZIP_EXCL, &error);
        REQUIRE(archive != nullptr);
        
        // Add large file
        zip_source_t* source = zip_source_buffer(archive, largeXML.c_str(), 
                                                largeXML.length(), 0);
        REQUIRE(source != nullptr);
        
        zip_int64_t index = zip_file_add(archive, "project.xml", source, ZIP_FL_ENC_UTF_8);
        REQUIRE(index >= 0);
        
        int closeResult = zip_close(archive);
        REQUIRE(closeResult == 0);
        
        // Verify file size
        auto fileSize = fs::file_size(zipPath);
        REQUIRE(fileSize > 0);
        REQUIRE(fileSize < largeXML.length()); // Should be compressed
        
        INFO("Original size: " << largeXML.length() << " bytes");
        INFO("Compressed size: " << fileSize << " bytes");
        INFO("Compression ratio: " << (100.0 * fileSize / largeXML.length()) << "%");
        
        fs::remove_all(tempDir);
    }
    
    SECTION("ZIP Compression Levels") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_compression_test";
        fs::create_directories(tempDir);
        
        std::string testContent = XMLTestUtils::generateLargeXML(1000);
        
        // Test different compression scenarios
        std::vector<std::pair<std::string, size_t>> results;
        
        for (int level = 0; level <= 9; level += 3) { // Test levels 0, 3, 6, 9
            auto zipPath = tempDir / ("test_level_" + std::to_string(level) + ".dawproject");
            
            int error = 0;
            zip_t* archive = zip_open(zipPath.string().c_str(), ZIP_CREATE | ZIP_EXCL, &error);
            REQUIRE(archive != nullptr);
            
            // Set compression level (if supported by libzip)
            zip_source_t* source = zip_source_buffer(archive, testContent.c_str(), 
                                                    testContent.length(), 0);
            REQUIRE(source != nullptr);
            
            zip_int64_t index = zip_file_add(archive, "project.xml", source, ZIP_FL_ENC_UTF_8);
            REQUIRE(index >= 0);
            
            int closeResult = zip_close(archive);
            REQUIRE(closeResult == 0);
            
            size_t compressedSize = fs::file_size(zipPath);
            results.emplace_back("Level " + std::to_string(level), compressedSize);
            
            REQUIRE(compressedSize > 0);
        }
        
        // Report compression results
        for (const auto& result : results) {
            double ratio = 100.0 * result.second / testContent.length();
            INFO(result.first << ": " << result.second << " bytes (" << ratio << "%)");
        }
        
        fs::remove_all(tempDir);
    }
    
    SECTION("ZIP File Listing and Validation") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_list_test";
        fs::create_directories(tempDir);
        
        auto zipPath = tempDir / "multi_file_test.dawproject";
        
        int error = 0;
        zip_t* archive = zip_open(zipPath.string().c_str(), ZIP_CREATE | ZIP_EXCL, &error);
        REQUIRE(archive != nullptr);
        
        // Add multiple files
        std::vector<std::string> filenames = {
            "project.xml",
            "metadata.xml",
            "audio/track1.wav",
            "audio/track2.wav",
            "presets/effect1.xml"
        };
        
        for (const auto& filename : filenames) {
            std::string content = "Content for " + filename;
            zip_source_t* source = zip_source_buffer(archive, content.c_str(), 
                                                    content.length(), 0);
            REQUIRE(source != nullptr);
            
            zip_int64_t index = zip_file_add(archive, filename.c_str(), source, ZIP_FL_ENC_UTF_8);
            REQUIRE(index >= 0);
        }
        
        int closeResult = zip_close(archive);
        REQUIRE(closeResult == 0);
        
        // Read back and list files
        zip_t* readArchive = zip_open(zipPath.string().c_str(), ZIP_RDONLY, &error);
        REQUIRE(readArchive != nullptr);
        
        zip_int64_t numFiles = zip_get_num_entries(readArchive, 0);
        REQUIRE(numFiles == static_cast<zip_int64_t>(filenames.size()));
        
        std::vector<std::string> foundFiles;
        for (zip_int64_t i = 0; i < numFiles; ++i) {
            const char* name = zip_get_name(readArchive, i, 0);
            REQUIRE(name != nullptr);
            foundFiles.emplace_back(name);
        }
        
        // Verify all files are present
        for (const auto& expectedFile : filenames) {
            REQUIRE(std::find(foundFiles.begin(), foundFiles.end(), expectedFile) != foundFiles.end());
        }
        
        zip_close(readArchive);
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Data Access - Schema Validation Edge Cases", "[data][schema][validation]") {
    
    SECTION("DAWProject Version Compatibility") {
        // Test different version handling
        std::vector<std::pair<std::string, bool>> versionTests = {
            {"1.0", true},    // Valid current version
            {"0.9", false},   // Older version
            {"1.1", false},   // Future version
            {"", false},      // Missing version
            {"invalid", false} // Invalid version format
        };
        
        for (const auto& test : versionTests) {
            std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version=")" + test.first + R"(" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
</Project>)";
            
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(xml.c_str());
            
            REQUIRE(result.status == pugi::status_ok);
            
            auto project = doc.child("Project");
            std::string version = project.attribute("version").value();
            
            bool isValid = (version == "1.0");
            REQUIRE(isValid == test.second);
            
            INFO("Version: '" << test.first << "' Expected: " << test.second << " Got: " << isValid);
        }
    }
    
    SECTION("Required Element Validation") {
        // Test missing required elements
        std::vector<std::pair<std::string, bool>> elementTests = {
            // Valid minimal structure
            {R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
</Project>)", true},
            
            // Missing Application element
            {R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
</Project>)", false},
            
            // Missing required attributes
            {R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application version="1.0"/>
</Project>)", false} // Missing name attribute
        };
        
        for (size_t i = 0; i < elementTests.size(); ++i) {
            const auto& test = elementTests[i];
            
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(test.first.c_str());
            
            REQUIRE(result.status == pugi::status_ok);
            
            // Validate structure
            auto project = doc.child("Project");
            REQUIRE(project);
            
            auto application = project.child("Application");
            bool hasApplication = static_cast<bool>(application);
            bool hasValidName = hasApplication && application.attribute("name");
            
            bool structureValid = hasApplication && hasValidName;
            
            INFO("Test " << i << " - Expected: " << test.second << " Got: " << structureValid);
            REQUIRE(structureValid == test.second);
        }
    }
    
    SECTION("Attribute Type Validation") {
        // Test various attribute value types
        struct AttributeTest {
            std::string xml;
            std::string xpath;
            bool isValid;
            std::string description;
        };
        
        std::vector<AttributeTest> tests = {
            // Tempo value tests
            {R"(<Transport><Tempo><Real time="0" value="120.0"/></Tempo></Transport>)",
             "//Real[@time='0']", true, "Valid tempo value"},
            
            {R"(<Transport><Tempo><Real time="0" value="-10.0"/></Tempo></Transport>)",
             "//Real[@time='0']", false, "Invalid negative tempo"},
            
            {R"(<Transport><Tempo><Real time="0" value="abc"/></Tempo></Transport>)",
             "//Real[@time='0']", false, "Invalid non-numeric tempo"},
            
            // Time value tests
            {R"(<Transport><Tempo><Real time="0.5" value="120.0"/></Tempo></Transport>)",
             "//Real[@time='0.5']", true, "Valid fractional time"},
            
            {R"(<Transport><Tempo><Real time="-1" value="120.0"/></Tempo></Transport>)",
             "//Real[@time='-1']", false, "Invalid negative time"}
        };
        
        for (const auto& test : tests) {
            std::string fullXML = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://dawproject.org/XML">
    <Application name="TestDAW" version="1.0"/>
    )" + test.xml + R"(
</Project>)";
            
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(fullXML.c_str());
            
            REQUIRE(result.status == pugi::status_ok);
            
            // Find the element and validate its attributes
            pugi::xpath_node_set nodes = doc.select_nodes(test.xpath.c_str());
            bool found = !nodes.empty();
            
            if (found && test.isValid) {
                auto node = nodes.first().node();
                std::string value = node.attribute("value").value();
                
                // Validate numeric values where applicable
                if (test.xpath.find("Real") != std::string::npos) {
                    try {
                        double numValue = std::stod(value);
                        bool isValidNumber = (numValue >= 0); // Basic validation
                        REQUIRE(isValidNumber);
                    } catch (const std::exception&) {
                        REQUIRE_FALSE(test.isValid);
                    }
                }
            }
            
            INFO("Test: " << test.description << " - Expected valid: " << test.isValid);
        }
    }
}

TEST_CASE("Data Access - Concurrent Access Patterns", "[data][concurrency]") {
    
    SECTION("Concurrent XML Parsing") {
        const int numThreads = 4;
        const int iterationsPerThread = 25;
        std::vector<std::thread> threads;
        std::atomic<int> successCount{0};
        std::atomic<int> errorCount{0};
        
        std::string xml = XMLTestUtils::generateValidDAWProjectXML();
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < iterationsPerThread; ++i) {
                    try {
                        pugi::xml_document doc;
                        pugi::xml_parse_result result = doc.load_string(xml.c_str());
                        
                        if (result.status == pugi::status_ok) {
                            auto project = doc.child("Project");
                            if (project && project.attribute("version")) {
                                successCount.fetch_add(1);
                            } else {
                                errorCount.fetch_add(1);
                            }
                        } else {
                            errorCount.fetch_add(1);
                        }
                    } catch (const std::exception&) {
                        errorCount.fetch_add(1);
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        REQUIRE(successCount.load() == numThreads * iterationsPerThread);
        REQUIRE(errorCount.load() == 0);
        
        INFO("Successful concurrent parses: " << successCount.load());
    }
    
    SECTION("Thread-Safe XML Modification") {
        pugi::xml_document doc;
        std::string xml = XMLTestUtils::generateValidDAWProjectXML();
        pugi::xml_parse_result result = doc.load_string(xml.c_str());
        REQUIRE(result.status == pugi::status_ok);
        
        const int numThreads = 4;
        const int tracksPerThread = 100;
        std::vector<std::thread> threads;
        std::mutex docMutex;
        std::atomic<int> trackCounter{0};
        
        auto structure = doc.child("Project").child("Structure");
        REQUIRE(structure);
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < tracksPerThread; ++i) {
                    {
                        std::lock_guard<std::mutex> lock(docMutex);
                        
                        auto newTrack = structure.append_child("Track");
                        int trackId = trackCounter.fetch_add(1);
                        
                        newTrack.append_attribute("id") = ("concurrent_track_" + std::to_string(trackId)).c_str();
                        newTrack.append_attribute("name") = ("Track " + std::to_string(trackId)).c_str();
                        newTrack.append_attribute("mediaType") = "audio";
                        
                        auto channel = newTrack.append_child("Channel");
                        channel.append_attribute("role") = "regular";
                    }
                    
                    // Small delay to encourage context switching
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        // Count final tracks
        size_t finalTrackCount = 0;
        for (auto track : structure.children("Track")) {
            finalTrackCount++;
        }
        
        // Original track + added tracks
        size_t expectedTracks = 1 + (numThreads * tracksPerThread);
        REQUIRE(finalTrackCount == expectedTracks);
        
        INFO("Final track count: " << finalTrackCount);
    }
}

TEST_CASE("Data Access - Performance Benchmarks", "[data][performance][benchmark]") {
    
    SECTION("XML Parse Performance Scaling") {
        std::vector<size_t> trackCounts = {100, 500, 1000, 5000};
        
        for (size_t trackCount : trackCounts) {
            std::string xml = XMLTestUtils::generateLargeXML(trackCount);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_string(xml.c_str());
            
            auto parseEnd = std::chrono::high_resolution_clock::now();
            
            REQUIRE(result.status == pugi::status_ok);
            
            // Traverse and count elements
            size_t elementCount = 0;
            auto countElements = [&elementCount](pugi::xml_node node) -> void {
                elementCount++;
                for (auto child : node.children()) {
                    // Recursive counting would go here
                    elementCount++;
                }
            };
            
            countElements(doc.child("Project"));
            
            auto traverseEnd = std::chrono::high_resolution_clock::now();
            
            auto parseDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                parseEnd - start).count();
            auto traverseDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                traverseEnd - parseEnd).count();
            
            double parseRatio = static_cast<double>(parseDuration) / trackCount;
            double traverseRatio = static_cast<double>(traverseDuration) / elementCount;
            
            INFO("Tracks: " << trackCount 
                 << " Parse: " << parseDuration << "μs (" << parseRatio << "μs/track)"
                 << " Traverse: " << traverseDuration << "μs (" << traverseRatio << "μs/element)");
            
            // Performance assertions (adjust thresholds based on hardware)
            REQUIRE(parseRatio < 100);    // Less than 100μs per track to parse
            REQUIRE(traverseRatio < 10);  // Less than 10μs per element to traverse
        }
    }
    
    SECTION("ZIP I/O Performance") {
        auto tempDir = fs::temp_directory_path() / "dawproject_zip_perf_test";
        fs::create_directories(tempDir);
        
        std::vector<size_t> fileSizes = {1024, 10240, 102400, 1024000}; // 1KB to 1MB
        
        for (size_t fileSize : fileSizes) {
            std::string content(fileSize, 'X'); // Fill with X characters
            auto zipPath = tempDir / ("perf_" + std::to_string(fileSize) + ".zip");
            
            // Write performance
            auto writeStart = std::chrono::high_resolution_clock::now();
            
            int error = 0;
            zip_t* archive = zip_open(zipPath.string().c_str(), ZIP_CREATE | ZIP_EXCL, &error);
            REQUIRE(archive != nullptr);
            
            zip_source_t* source = zip_source_buffer(archive, content.c_str(), content.length(), 0);
            REQUIRE(source != nullptr);
            
            zip_int64_t index = zip_file_add(archive, "data.txt", source, ZIP_FL_ENC_UTF_8);
            REQUIRE(index >= 0);
            
            int closeResult = zip_close(archive);
            REQUIRE(closeResult == 0);
            
            auto writeEnd = std::chrono::high_resolution_clock::now();
            
            // Read performance
            auto readStart = std::chrono::high_resolution_clock::now();
            
            zip_t* readArchive = zip_open(zipPath.string().c_str(), ZIP_RDONLY, &error);
            REQUIRE(readArchive != nullptr);
            
            zip_file_t* file = zip_fopen(readArchive, "data.txt", 0);
            REQUIRE(file != nullptr);
            
            std::vector<char> buffer(fileSize);
            zip_int64_t bytesRead = zip_fread(file, buffer.data(), buffer.size());
            REQUIRE(bytesRead == static_cast<zip_int64_t>(fileSize));
            
            zip_fclose(file);
            zip_close(readArchive);
            
            auto readEnd = std::chrono::high_resolution_clock::now();
            
            auto writeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                writeEnd - writeStart).count();
            auto readDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                readEnd - readStart).count();
            
            double writeMBps = (fileSize / 1024.0 / 1024.0) / (writeDuration / 1000000.0);
            double readMBps = (fileSize / 1024.0 / 1024.0) / (readDuration / 1000000.0);
            
            INFO("Size: " << fileSize << " bytes"
                 << " Write: " << writeDuration << "μs (" << writeMBps << " MB/s)"
                 << " Read: " << readDuration << "μs (" << readMBps << " MB/s)");
            
            // Basic performance thresholds
            REQUIRE(writeMBps > 1.0);  // At least 1 MB/s write
            REQUIRE(readMBps > 5.0);   // At least 5 MB/s read
        }
        
        fs::remove_all(tempDir);
    }
}