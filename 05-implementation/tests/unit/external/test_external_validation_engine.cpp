#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "dawproject/external/external_validation_engine.h"
#include "dawproject/external/validation_models.h"
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace dawproject::external;
using namespace dawproject;

// Test data constants
namespace {
    const std::string VALID_DAWPROJECT_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://www.bitwig.com/dawproject">
    <MetaData author="Test Author" />
    <Transport>
        <Tempo value="120" />
    </Transport>
</Project>)";

    const std::string INVALID_DAWPROJECT_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project xmlns="http://www.bitwig.com/dawproject">
    <MetaData author="Test Author" />
</Project>)"; // Missing required version attribute

    const std::string MALFORMED_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://www.bitwig.com/dawproject">
    <MetaData author="Test Author" />
    <UnclosedTag>
</Project>)";

    const std::string VALID_METADATA_XML = R"(<?xml version="1.0" encoding="UTF-8"?>
<MetaData xmlns="http://www.bitwig.com/dawproject" 
          author="Test Author" 
          name="Test Project" 
          version="1.0">
    <Description>Test metadata</Description>
</MetaData>)";
}

TEST_CASE("ExternalValidationEngine can be created with factory", "[external][validation][factory]") {
    SECTION("Create with default configuration") {
        auto engine = ExternalValidationEngineFactory::create();
        REQUIRE(engine != nullptr);
    }
    
    SECTION("Create with custom configuration") {
        ExternalValidationConfig config;
        config.enableXMLSecurity = true;
        config.schemaTimeout = std::chrono::seconds(60);
        config.enableCaching = true;
        
        auto engine = ExternalValidationEngineFactory::create(config);
        REQUIRE(engine != nullptr);
    }
}

TEST_CASE("ExternalValidationEngine provides engine information", "[external][validation][info]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto infoResult = engine->getEngineInfo();
    REQUIRE(infoResult.isSuccess());
    
    auto info = infoResult.value();
    REQUIRE_FALSE(info.engineVersion.empty());
    REQUIRE_FALSE(info.libxml2Version.empty());
    REQUIRE(info.externalAuthoritySupport == true);
    REQUIRE_FALSE(info.supportedSchemaTypes.empty());
}

TEST_CASE("ExternalValidationEngine checks schema availability", "[external][validation][schema]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto availabilityResult = engine->checkSchemaAvailability();
    
    // This test may fail if network is unavailable - that's expected behavior
    if (availabilityResult.isSuccess()) {
        auto availability = availabilityResult.value();
        REQUIRE_FALSE(availability.projectSchemaUrl.empty());
        REQUIRE_FALSE(availability.metaDataSchemaUrl.empty());
        
        // If schemas are available, they should be marked as such
        if (availability.projectSchemaAvailable) {
            INFO("Project schema is available from: " << availability.projectSchemaUrl);
        }
        if (availability.metaDataSchemaAvailable) {
            INFO("MetaData schema is available from: " << availability.metaDataSchemaUrl);
        }
    } else {
        // Network failure is acceptable - log for debugging
        INFO("Schema availability check failed (network may be unavailable): " 
             << availabilityResult.error().message);
    }
}

TEST_CASE("ExternalValidationEngine validates valid DAWProject XML", "[external][validation][project][valid]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto validationResult = engine->validateProjectXML(VALID_DAWPROJECT_XML);
    
    // The validation operation should succeed (no system errors)
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    
    // Check report structure
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    REQUIRE_FALSE(report.schemaPath.empty());
    REQUIRE_FALSE(report.externalSchemaSource.empty());
    
    // Performance requirements: <2s for files <1MB
    REQUIRE(report.validationTime <= std::chrono::seconds(2));
    
    // Document should be valid (assuming external schema is available)
    if (report.isValid) {
        REQUIRE(report.totalErrorCount == 0);
        INFO("Valid XML passed validation successfully");
    } else {
        INFO("Valid XML failed validation - may indicate schema unavailability");
        for (const auto& error : report.errors) {
            INFO("Validation error: " << error.message);
        }
    }
}

TEST_CASE("ExternalValidationEngine detects invalid DAWProject XML", "[external][validation][project][invalid]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto validationResult = engine->validateProjectXML(INVALID_DAWPROJECT_XML);
    
    // The validation operation should succeed (no system errors)
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    
    // Check report structure
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    
    // Document should be invalid due to missing version attribute
    if (report.externalAuthorityCompliant) {
        REQUIRE_FALSE(report.isValid);
        REQUIRE(report.totalErrorCount > 0);
        
        // Should detect missing version attribute
        bool foundVersionError = false;
        for (const auto& error : report.errors) {
            if (error.message.find("version") != std::string::npos) {
                foundVersionError = true;
                break;
            }
        }
        REQUIRE(foundVersionError);
    } else {
        INFO("External schema not available - skipping validation accuracy test");
    }
}

TEST_CASE("ExternalValidationEngine handles malformed XML", "[external][validation][malformed]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto validationResult = engine->validateProjectXML(MALFORMED_XML);
    
    // The validation operation should succeed (no system errors)
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    
    // Malformed XML should be detected as invalid
    REQUIRE_FALSE(report.isValid);
    REQUIRE(report.totalErrorCount > 0);
    
    // Should detect XML parse error
    bool foundParseError = false;
    for (const auto& error : report.errors) {
        if (error.message.find("parse") != std::string::npos || 
            error.message.find("tag") != std::string::npos) {
            foundParseError = true;
            break;
        }
    }
    REQUIRE(foundParseError);
}

TEST_CASE("ExternalValidationEngine validates metadata XML", "[external][validation][metadata]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    auto validationResult = engine->validateMetaDataXML(VALID_METADATA_XML);
    
    // The validation operation should succeed (no system errors)  
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    
    // Check report structure
    REQUIRE(report.schemaType == SchemaType::MetaDataSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    
    // Performance requirement: <1s for metadata
    REQUIRE(report.validationTime <= std::chrono::seconds(1));
}

TEST_CASE("ExternalValidationEngine validates XML file", "[external][validation][file]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    // Create temporary XML file
    std::filesystem::path tempFile = std::filesystem::temp_directory_path() / "test_project.xml";
    
    {
        std::ofstream file(tempFile);
        file << VALID_DAWPROJECT_XML;
    }
    
    auto validationResult = engine->validateXMLFile(tempFile, SchemaType::ProjectSchema);
    
    // Cleanup
    std::filesystem::remove(tempFile);
    
    // The validation operation should succeed
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
}

TEST_CASE("ExternalValidationEngine supports batch validation", "[external][validation][batch]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    std::vector<ValidationRequest> requests;
    
    // Add multiple validation requests
    requests.push_back({VALID_DAWPROJECT_XML, SchemaType::ProjectSchema, "valid-test", std::nullopt});
    requests.push_back({INVALID_DAWPROJECT_XML, SchemaType::ProjectSchema, "invalid-test", std::nullopt});
    requests.push_back({VALID_METADATA_XML, SchemaType::MetaDataSchema, "metadata-test", std::nullopt});
    
    auto batchResult = engine->batchValidate(requests);
    
    // The batch validation operation should succeed
    REQUIRE(batchResult.isSuccess());
    
    auto reports = batchResult.value();
    REQUIRE(reports.size() == requests.size());
    
    // Verify each report corresponds to the correct request
    REQUIRE(reports[0].schemaType == SchemaType::ProjectSchema);
    REQUIRE(reports[1].schemaType == SchemaType::ProjectSchema);
    REQUIRE(reports[2].schemaType == SchemaType::MetaDataSchema);
}

TEST_CASE("ExternalValidationEngine updates configuration", "[external][validation][config]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    ExternalValidationConfig newConfig;
    newConfig.enableXMLSecurity = false;
    newConfig.schemaTimeout = std::chrono::seconds(120);
    newConfig.enableDetailedErrors = true;
    
    auto updateResult = engine->updateConfiguration(newConfig);
    REQUIRE(updateResult.isSuccess());
}

// Performance test for validation requirements
TEST_CASE("ExternalValidationEngine meets performance requirements", "[external][validation][performance]") {
    auto engine = ExternalValidationEngineFactory::create();
    REQUIRE(engine != nullptr);
    
    // Create larger XML content (simulate project with multiple tracks)
    std::string largeXML = R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version="1.0" xmlns="http://www.bitwig.com/dawproject">
    <MetaData author="Performance Test" />
    <Transport>
        <Tempo value="120" />
    </Transport>)";
    
    // Add multiple tracks to increase size
    for (int i = 0; i < 50; ++i) {
        largeXML += R"(
    <Track id=")" + std::to_string(i) + R"(" name="Track )" + std::to_string(i) + R"(">
        <Channel>
            <Volume value="0.8" />
            <Pan value="0.0" />
        </Channel>
    </Track>)";
    }
    largeXML += "\n</Project>";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto validationResult = engine->validateProjectXML(largeXML);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance requirement: <2s for files <1MB
    REQUIRE(duration <= std::chrono::seconds(2));
    
    if (validationResult.isSuccess()) {
        auto report = validationResult.value();
        INFO("Validation completed in " << duration.count() << "ms");
        INFO("Document size: " << largeXML.size() << " bytes");
        INFO("Reported validation time: " << report.validationTime.count() << "ms");
    }
}