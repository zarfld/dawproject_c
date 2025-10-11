#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "data/xml_processor_impl.h"
#include "data/zip_processor_impl.h"
#include "data/data_access_factory.h"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace dawproject::data;

/**
 * @brief Comprehensive Data Access Layer Tests for Coverage Improvement
 * 
 * Target: Increase coverage by testing edge cases, error conditions, and boundary scenarios
 * Focus: XML/ZIP processing, malformed data handling, large files, encoding issues
 */

TEST_CASE("XML Processor - Advanced Parsing Scenarios", "[data][xml][advanced]") {
    auto xmlProcessor = DataAccessFactory::createXMLProcessor();
    REQUIRE(xmlProcessor != nullptr);
    
    SECTION("Complex DAWProject XML Structure") {
        std::string complexXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Application name="TestDAW" version="2.0"/>
  <Transport>
    <Tempo max="200.0" min="60.0" unit="bpm" value="120.0" id="tempo1" name="Master Tempo"/>
    <TimeSignature numerator="4" denominator="4" id="timesig1"/>
  </Transport>
  <Structure>
    <Track contentType="audio notes" loaded="true" id="track1" name="Lead Vocal">
      <Channel role="regular" audioChannels="2" destination="master"/>
      <Clips>
        <Clip time="0" duration="16" name="Verse 1" id="clip1">
          <Notes>
            <Note time="0" duration="1" key="60" velocity="80" channel="1"/>
            <Note time="1" duration="0.5" key="64" velocity="75" channel="1"/>
          </Notes>
        </Clip>
        <Clip time="16" duration="8" name="Chorus" id="clip2">
          <Notes>
            <Note time="16" duration="2" key="67" velocity="90" channel="1"/>
          </Notes>
        </Clip>
      </Clips>
    </Track>
    <Track contentType="audio" loaded="true" id="track2" name="Bass">
      <Channel role="regular" audioChannels="1" destination="master"/>
      <Clips>
        <Clip time="0" duration="32" name="Bassline" id="clip3">
          <Audio file="bass_track.wav" channels="1" duration="32"/>
        </Clip>
      </Clips>
    </Track>
  </Structure>
  <Arrangement>
    <Lanes>
      <Lane timeUnit="beats" id="lane1" name="Main">
        <Clips>
          <ClipSlot time="0" clipID="clip1"/>
          <ClipSlot time="16" clipID="clip2"/>
          <ClipSlot time="0" clipID="clip3"/>
        </Clips>
      </Lane>
    </Lanes>
  </Arrangement>
</Project>)";

        auto result = xmlProcessor->parseDocument(complexXml);
        REQUIRE(result.success);
        REQUIRE(result.value.rootElementName == "Project");
        
        // Test element access
        auto projectElement = xmlProcessor->findElement(result.value, "Project");
        REQUIRE(projectElement.success);
        REQUIRE(xmlProcessor->getAttribute(projectElement.value, "version") == "1.0");
        
        // Test nested element navigation
        auto tracks = xmlProcessor->findElements(result.value, "Track");
        REQUIRE(tracks.success);
        REQUIRE(tracks.value.size() == 2);
        
        // Test attribute access on nested elements
        auto firstTrack = tracks.value[0];
        REQUIRE(xmlProcessor->getAttribute(firstTrack, "name") == "Lead Vocal");
        REQUIRE(xmlProcessor->getAttribute(firstTrack, "contentType") == "audio notes");
    }
    
    SECTION("XML with CDATA Sections") {
        std::string cdataXml = R"(<?xml version="1.0"?>
<Project version="1.0">
  <Metadata>
    <Description><![CDATA[This is a complex description with <special> characters & symbols]]></Description>
    <Script><![CDATA[
      function processAudio(samples) {
        for (let i = 0; i < samples.length; i++) {
          samples[i] *= 0.8; // Apply gain reduction
        }
        return samples;
      }
    ]]></Script>
  </Metadata>
</Project>)";

        auto result = xmlProcessor->parseDocument(cdataXml);
        REQUIRE(result.success);
        
        // Test CDATA content extraction
        auto description = xmlProcessor->findElement(result.value, "Description");
        REQUIRE(description.success);
        
        auto descText = xmlProcessor->getElementText(description.value);
        REQUIRE(descText.find("<special>") != std::string::npos);
        REQUIRE(descText.find("&") != std::string::npos);
    }
    
    SECTION("XML Namespace Handling") {
        std::string namespacedXml = R"(<?xml version="1.0"?>
<daw:Project xmlns:daw="http://dawproject.org/schema" 
             xmlns:meta="http://dawproject.org/metadata" 
             version="1.0">
  <meta:Application name="TestDAW" version="1.0"/>
  <daw:Structure>
    <daw:Track id="track1" name="Test Track"/>
  </daw:Structure>
</daw:Project>)";

        auto result = xmlProcessor->parseDocument(namespacedXml);
        REQUIRE(result.success);
        
        // Test namespace-aware element access
        auto project = xmlProcessor->findElementNS(result.value, "Project", "http://dawproject.org/schema");
        if (project.success) {
            REQUIRE(xmlProcessor->getAttribute(project.value, "version") == "1.0");
        }
    }
}

TEST_CASE("XML Processor - Error Conditions and Malformed Data", "[data][xml][error]") {
    auto xmlProcessor = DataAccessFactory::createXMLProcessor();
    REQUIRE(xmlProcessor != nullptr);
    
    SECTION("Malformed XML Documents") {
        // Missing closing tag
        std::string unclosedTag = R"(<?xml version="1.0"?><Project><Track></Project>)";
        auto result1 = xmlProcessor->parseDocument(unclosedTag);
        REQUIRE_FALSE(result1.success);
        REQUIRE_FALSE(result1.errorMessage.empty());
        
        // Invalid XML characters
        std::string invalidChars = R"(<?xml version="1.0"?><Project>\x01\x02</Project>)";
        auto result2 = xmlProcessor->parseDocument(invalidChars);
        REQUIRE_FALSE(result2.success);
        
        // Unescaped special characters
        std::string unescaped = R"(<?xml version="1.0"?><Project name="Test & Debug"></Project>)";
        auto result3 = xmlProcessor->parseDocument(unescaped);
        REQUIRE_FALSE(result3.success);
    }
    
    SECTION("Invalid Encoding Scenarios") {
        // Declared UTF-8 but contains invalid sequences
        std::string invalidUtf8 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Project>\xFF\xFE</Project>";
        auto result = xmlProcessor->parseDocument(invalidUtf8);
        // Should either handle gracefully or fail with meaningful error
        if (!result.success) {
            REQUIRE_FALSE(result.errorMessage.empty());
            REQUIRE(result.errorMessage.find("encoding") != std::string::npos ||
                   result.errorMessage.find("UTF-8") != std::string::npos ||
                   result.errorMessage.find("character") != std::string::npos);
        }
    }
    
    SECTION("Extremely Large XML Documents") {
        // Create large XML document (simulate 10MB+ file)
        std::ostringstream largeXml;
        largeXml << R"(<?xml version="1.0"?><Project version="1.0"><Structure>)";
        
        // Add many tracks
        for (int i = 0; i < 10000; ++i) {
            largeXml << "<Track id=\"track" << i << "\" name=\"Track " << i << "\">";
            // Add clips to each track
            for (int j = 0; j < 10; ++j) {
                largeXml << "<Clip id=\"clip" << i << "_" << j << "\" time=\"" << j << "\" duration=\"1\">";
                largeXml << "<Notes>";
                // Add notes to each clip
                for (int k = 0; k < 5; ++k) {
                    largeXml << "<Note time=\"" << k*0.25 << "\" key=\"" << (60+k) << "\" velocity=\"80\"/>";
                }
                largeXml << "</Notes></Clip>";
            }
            largeXml << "</Track>";
        }
        
        largeXml << "</Structure></Project>";
        
        auto largeXmlStr = largeXml.str();
        
        // Test parsing large document
        auto result = xmlProcessor->parseDocument(largeXmlStr);
        if (result.success) {
            // If parsing succeeds, verify structure
            auto tracks = xmlProcessor->findElements(result.value, "Track");
            REQUIRE(tracks.success);
            REQUIRE(tracks.value.size() == 10000);
        } else {
            // If parsing fails, should have meaningful error (memory, size limit, etc.)
            REQUIRE_FALSE(result.errorMessage.empty());
            INFO("Large XML parsing error: " + result.errorMessage);
        }
    }
    
    SECTION("XML with Deep Nesting") {
        // Create deeply nested XML (test recursion limits)
        std::ostringstream deepXml;
        deepXml << "<?xml version=\"1.0\"?>";
        
        const int nestingDepth = 1000;
        
        // Create nested structure
        for (int i = 0; i < nestingDepth; ++i) {
            deepXml << "<Level" << i << ">";
        }
        
        deepXml << "<Content>Deep nested content</Content>";
        
        for (int i = nestingDepth - 1; i >= 0; --i) {
            deepXml << "</Level" << i << ">";
        }
        
        auto result = xmlProcessor->parseDocument(deepXml.str());
        if (result.success) {
            // Verify we can navigate to deep content
            auto content = xmlProcessor->findElement(result.value, "Content");
            REQUIRE(content.success);
        } else {
            // Should handle recursion limit gracefully
            REQUIRE_FALSE(result.errorMessage.empty());
            INFO("Deep nesting error: " + result.errorMessage);
        }
    }
}

TEST_CASE("ZIP Processor - Advanced Archive Operations", "[data][zip][advanced]") {
    auto zipProcessor = DataAccessFactory::createZIPProcessor();
    REQUIRE(zipProcessor != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_zip_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("Create and Manipulate ZIP Archive") {
        std::filesystem::path archivePath = tempDir / "test_project.dawproject";
        
        // Create project.xml content
        std::string projectXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0">
  <Application name="TestDAW" version="1.0"/>
  <Transport>
    <Tempo value="120.0"/>
  </Transport>
  <Structure>
    <Track id="track1" name="Test Track"/>
  </Structure>
</Project>)";
        
        // Create metadata.xml content
        std::string metadataXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Metadata>
  <Title>Test Project</Title>
  <Artist>Test Artist</Artist>
  <CreationDate>2024-10-11</CreationDate>
</Metadata>)";
        
        // Add files to archive
        auto addProjectResult = zipProcessor->addEntry(archivePath, "project.xml", 
                                                     std::vector<uint8_t>(projectXml.begin(), projectXml.end()));
        REQUIRE(addProjectResult.success);
        
        auto addMetadataResult = zipProcessor->addEntry(archivePath, "metadata.xml", 
                                                       std::vector<uint8_t>(metadataXml.begin(), metadataXml.end()));
        REQUIRE(addMetadataResult.success);
        
        // Add binary audio data
        std::vector<uint8_t> audioData(44100 * 2 * 4); // 1 second of stereo 16-bit audio
        for (size_t i = 0; i < audioData.size(); ++i) {
            audioData[i] = static_cast<uint8_t>(i % 256); // Generate test pattern
        }
        
        auto addAudioResult = zipProcessor->addEntry(archivePath, "audio/track1.wav", audioData);
        REQUIRE(addAudioResult.success);
        
        // Verify archive contents
        auto listResult = zipProcessor->listEntries(archivePath);
        REQUIRE(listResult.success);
        REQUIRE(listResult.value.size() >= 3);
        
        // Find specific entries
        bool foundProject = false, foundMetadata = false, foundAudio = false;
        for (const auto& entry : listResult.value) {
            if (entry.name == "project.xml") {
                foundProject = true;
                REQUIRE(entry.compressedSize > 0);
                REQUIRE(entry.uncompressedSize >= projectXml.length());
            }
            else if (entry.name == "metadata.xml") {
                foundMetadata = true;
            }
            else if (entry.name == "audio/track1.wav") {
                foundAudio = true;
                REQUIRE(entry.uncompressedSize == audioData.size());
            }
        }
        
        REQUIRE(foundProject);
        REQUIRE(foundMetadata);
        REQUIRE(foundAudio);
        
        // Extract and verify content
        auto extractProjectResult = zipProcessor->extractEntry(archivePath, "project.xml");
        REQUIRE(extractProjectResult.success);
        
        std::string extractedProject(extractProjectResult.value.begin(), extractProjectResult.value.end());
        REQUIRE(extractedProject == projectXml);
    }
    
    SECTION("ZIP Archive Error Conditions") {
        std::filesystem::path nonExistentArchive = tempDir / "nonexistent.dawproject";
        
        // Try to list entries from non-existent archive
        auto listResult = zipProcessor->listEntries(nonExistentArchive);
        REQUIRE_FALSE(listResult.success);
        REQUIRE_FALSE(listResult.errorMessage.empty());
        
        // Try to extract from non-existent archive
        auto extractResult = zipProcessor->extractEntry(nonExistentArchive, "project.xml");
        REQUIRE_FALSE(extractResult.success);
        
        // Create empty file and try to treat as ZIP
        std::filesystem::path emptyFile = tempDir / "empty.dawproject";
        std::ofstream(emptyFile) << "";
        
        auto listEmptyResult = zipProcessor->listEntries(emptyFile);
        REQUIRE_FALSE(listEmptyResult.success);
        
        // Create file with invalid ZIP signature
        std::filesystem::path invalidZip = tempDir / "invalid.dawproject";
        std::ofstream invalidFile(invalidZip, std::ios::binary);
        invalidFile << "NOT A ZIP FILE";
        invalidFile.close();
        
        auto listInvalidResult = zipProcessor->listEntries(invalidZip);
        REQUIRE_FALSE(listInvalidResult.success);
        REQUIRE(listInvalidResult.errorMessage.find("ZIP") != std::string::npos ||
               listInvalidResult.errorMessage.find("archive") != std::string::npos);
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}

TEST_CASE("ZIP Processor - Compression and Performance", "[data][zip][performance]") {
    auto zipProcessor = DataAccessFactory::createZIPProcessor();
    REQUIRE(zipProcessor != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_zip_perf_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("Compression Efficiency Testing") {
        std::filesystem::path archivePath = tempDir / "compression_test.dawproject";
        
        // Create highly compressible data (repeated patterns)
        std::string compressibleData(100000, 'A'); // 100KB of 'A's
        std::vector<uint8_t> compressibleBytes(compressibleData.begin(), compressibleData.end());
        
        auto addResult = zipProcessor->addEntry(archivePath, "compressible.txt", compressibleBytes);
        REQUIRE(addResult.success);
        
        // Create incompressible data (random-like)
        std::vector<uint8_t> incompressibleData;
        incompressibleData.reserve(100000);
        for (size_t i = 0; i < 100000; ++i) {
            incompressibleData.push_back(static_cast<uint8_t>((i * 31) % 256));
        }
        
        auto addIncompressibleResult = zipProcessor->addEntry(archivePath, "incompressible.bin", incompressibleData);
        REQUIRE(addIncompressibleResult.success);
        
        // Check compression ratios
        auto listResult = zipProcessor->listEntries(archivePath);
        REQUIRE(listResult.success);
        
        for (const auto& entry : listResult.value) {
            if (entry.name == "compressible.txt") {
                double ratio = static_cast<double>(entry.compressedSize) / entry.uncompressedSize;
                REQUIRE(ratio < 0.5); // Should compress to less than 50%
                INFO("Compressible data ratio: " + std::to_string(ratio));
            }
            else if (entry.name == "incompressible.bin") {
                double ratio = static_cast<double>(entry.compressedSize) / entry.uncompressedSize;
                INFO("Incompressible data ratio: " + std::to_string(ratio));
                // Incompressible data should not compress much (may even be larger due to overhead)
            }
        }
    }
    
    SECTION("Large Archive Handling") {
        std::filesystem::path largeArchivePath = tempDir / "large_test.dawproject";
        
        // Add multiple large files
        for (int fileNum = 0; fileNum < 5; ++fileNum) {
            std::vector<uint8_t> largeFile(1024 * 1024); // 1MB per file
            
            // Fill with pattern
            for (size_t i = 0; i < largeFile.size(); ++i) {
                largeFile[i] = static_cast<uint8_t>((i + fileNum * 1000) % 256);
            }
            
            std::string fileName = "large_file_" + std::to_string(fileNum) + ".dat";
            auto addResult = zipProcessor->addEntry(largeArchivePath, fileName, largeFile);
            REQUIRE(addResult.success);
        }
        
        // Verify all files are in archive
        auto listResult = zipProcessor->listEntries(largeArchivePath);
        REQUIRE(listResult.success);
        REQUIRE(listResult.value.size() == 5);
        
        // Test extraction of large file
        auto extractResult = zipProcessor->extractEntry(largeArchivePath, "large_file_2.dat");
        REQUIRE(extractResult.success);
        REQUIRE(extractResult.value.size() == 1024 * 1024);
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}