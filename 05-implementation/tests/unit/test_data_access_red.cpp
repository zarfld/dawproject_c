#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <dawproject/data/data_access_engine.h>
#include <dawproject/data/data_access_factory.h>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace dawproject::data;
using namespace std::filesystem;

TEST_CASE("Data Access Factory - Create Engine", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    REQUIRE(engine != nullptr);
    INFO("Factory should create a valid data access engine instance");
}

TEST_CASE("Data Access Factory - Create XML Processor", "[data-access]") {
    auto xmlProcessor = DataAccessFactory::createXMLProcessor();
    
    REQUIRE(xmlProcessor != nullptr);
    INFO("Factory should create a valid XML processor instance");
}

TEST_CASE("Data Access Factory - Create ZIP Processor", "[data-access]") {
    auto zipProcessor = DataAccessFactory::createZIPProcessor();
    
    REQUIRE(zipProcessor != nullptr);
    INFO("Factory should create a valid ZIP processor instance");
}

TEST_CASE("Project Info - Validation", "[data-access]") {
    ProjectInfo validProject;
    validProject.title = "Test Project";
    validProject.artist = "Test Artist";
    validProject.tempo = 120.0;
    validProject.timeSignature = "4/4";
    
    REQUIRE(validProject.isValid());
    REQUIRE(validProject.getValidationErrors().empty());
    
    ProjectInfo invalidProject;
    // Empty title should make it invalid
    invalidProject.artist = "Test Artist";
    invalidProject.tempo = 120.0;
    
    REQUIRE_FALSE(invalidProject.isValid());
    REQUIRE_FALSE(invalidProject.getValidationErrors().empty());
}

TEST_CASE("Track Info - Validation", "[data-access]") {
    TrackInfo validTrack;
    validTrack.id = "track-001";
    validTrack.name = "Test Track";
    validTrack.type = TrackType::Audio;
    validTrack.volume = 0.8;
    validTrack.pan = 0.0;
    validTrack.muted = false;
    validTrack.soloed = false;
    validTrack.orderIndex = 0;
    
    REQUIRE(validTrack.isValid());
    
    TrackInfo invalidTrack;
    // Empty ID should make it invalid
    invalidTrack.name = "Test Track";
    invalidTrack.type = TrackType::Audio;
    
    REQUIRE_FALSE(invalidTrack.isValid());
}

TEST_CASE("Clip Info - Validation", "[data-access]") {
    ClipInfo validClip;
    validClip.id = "clip-001";
    validClip.name = "Test Clip";
    validClip.trackId = "track-001";
    validClip.startTime = 0.0;
    validClip.duration = 4.0;
    validClip.playbackRate = 1.0;
    validClip.fadeInTime = 0.1;
    validClip.fadeOutTime = 0.1;
    
    REQUIRE(validClip.isValid());
    
    ClipInfo invalidClip;
    // Empty ID should make it invalid
    invalidClip.name = "Test Clip";
    invalidClip.trackId = "track-001";
    
    REQUIRE_FALSE(invalidClip.isValid());
}

TEST_CASE("Validation Result - Operations", "[data-access]") {
    ValidationResult result;
    result.isValid = true;
    result.context = "Test Context";
    
    result.addError("Test error message");
    REQUIRE_FALSE(result.isValid);
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0] == "Test error message");
    
    result.addWarning("Test warning message");
    REQUIRE(result.warnings.size() == 1);
    REQUIRE(result.warnings[0] == "Test warning message");
    
    ValidationResult other;
    other.addError("Other error");
    other.addWarning("Other warning");
    
    result.merge(other);
    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.warnings.size() == 2);
}

TEST_CASE("Result Template - Success Case", "[data-access]") {
    auto result = Result<int>::makeSuccess(42);
    
    REQUIRE(result.success);
    REQUIRE(result);
    REQUIRE(result.value == 42);
    REQUIRE(result.errorMessage.empty());
    REQUIRE(result.errorCode == 0);
}

TEST_CASE("Result Template - Error Case", "[data-access]") {
    auto result = Result<int>::makeError("Test error", 404);
    
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result);
    REQUIRE(result.value == 0);  // Default constructed
    REQUIRE(result.errorMessage == "Test error");
    REQUIRE(result.errorCode == 404);
}

TEST_CASE("Archive Entry - Validation", "[data-access]") {
    ArchiveEntry validEntry;
    validEntry.name = "project.xml";
    validEntry.path = "project.xml";
    validEntry.compressedSize = 1024;
    validEntry.uncompressedSize = 2048;
    validEntry.isDirectory = false;
    
    REQUIRE(validEntry.isValid());
    
    ArchiveEntry invalidEntry;
    // Empty name should make it invalid
    invalidEntry.path = "project.xml";
    
    REQUIRE_FALSE(invalidEntry.isValid());
}

TEST_CASE("XML Document - Helper Methods", "[data-access]") {
    XMLDocument doc;
    doc.rootElementName = "Project";
    doc.setAttribute("version", "1.0");
    
    REQUIRE(doc.getAttribute("version") == "1.0");
    REQUIRE(doc.getAttribute("nonexistent", "default") == "default");
    
    XMLDocument child;
    child.rootElementName = "Track";
    child.setAttribute("id", "track-001");
    doc.children.push_back(child);
    
    auto foundChild = doc.findChild("Track");
    REQUIRE(foundChild != nullptr);
    REQUIRE(foundChild->getAttribute("id") == "track-001");
    
    auto notFoundChild = doc.findChild("NonExistent");
    REQUIRE(notFoundChild == nullptr);
    
    auto foundChildren = doc.findChildren("Track");
    REQUIRE(foundChildren.size() == 1);
}

TEST_CASE("Data Access Engine - Load Project Info", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Test with non-existent file
    auto result = engine->loadProjectInfo("nonexistent.dawproject");
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("Data Access Engine - Load Tracks", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Test with non-existent file
    auto result = engine->loadTracks("nonexistent.dawproject");
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("Data Access Engine - Load Clips", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Test with non-existent file
    auto result = engine->loadClips("nonexistent.dawproject");
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("Data Access Engine - Validate File", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Test with non-existent file
    auto result = engine->validateFile("nonexistent.dawproject");
    REQUIRE_FALSE(result.isValid);
    REQUIRE_FALSE(result.errors.empty());
}

TEST_CASE("Data Access Engine - Is Valid Project File", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Test with non-existent file
    bool isValid = engine->isValidProjectFile("nonexistent.dawproject");
    REQUIRE_FALSE(isValid);
}

TEST_CASE("Data Access Engine - Create Reader", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Should create reader even for non-existent file
    auto reader = engine->createReader("test.dawproject");
    REQUIRE(reader != nullptr);
}

TEST_CASE("Data Access Engine - Create Writer", "[data-access]") {
    auto engine = DataAccessFactory::createDataAccessEngine();
    
    // Should create writer
    auto writer = engine->createWriter("test.dawproject");
    REQUIRE(writer != nullptr);
}

TEST_CASE("XML Processor - Load Document", "[data-access]") {
    auto processor = DataAccessFactory::createXMLProcessor();
    
    // Test with non-existent file
    auto result = processor->loadDocument("nonexistent.xml");
    REQUIRE_FALSE(result.success);
}

TEST_CASE("XML Processor - Parse Document", "[data-access]") {
    auto processor = DataAccessFactory::createXMLProcessor();
    
    // Test with valid XML
    std::string validXml = "<?xml version=\"1.0\"?><Project version=\"1.0\"></Project>";
    auto result = processor->parseDocument(validXml);
    REQUIRE(result.success);
    REQUIRE(result.value.rootElementName == "Project");
    
    // Test with invalid XML
    std::string invalidXml = "<Project><Track></Project>";  // Mismatched tags
    auto invalidResult = processor->parseDocument(invalidXml);
    REQUIRE_FALSE(invalidResult.success);
}

TEST_CASE("ZIP Processor - List Entries", "[data-access]") {
    auto processor = DataAccessFactory::createZIPProcessor();
    
    // Test with non-existent file
    auto result = processor->listEntries("nonexistent.zip");
    REQUIRE_FALSE(result.success);
}

TEST_CASE("ZIP Processor - Extract Entry", "[data-access]") {
    auto processor = DataAccessFactory::createZIPProcessor();
    
    // Test with non-existent file
    auto result = processor->extractEntry("nonexistent.zip", "project.xml");
    REQUIRE_FALSE(result.success);
}

TEST_CASE("ZIP Processor - Is Valid Archive", "[data-access]") {
    auto processor = DataAccessFactory::createZIPProcessor();
    
    // Test with non-existent file
    bool isValid = processor->isValidArchive("nonexistent.zip");
    REQUIRE_FALSE(isValid);
}