#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "dawproject/application/external-validation/external_validation_service.h"
#include "dawproject/domain/validation/validation_models.h"
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace dawproject::application::external_validation;
using namespace dawproject::domain::validation;
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

TEST_CASE("ExternalValidationService can be created with factory", "[application][external-validation][factory]") {
    SECTION("Create with default configuration") {
        auto service = ExternalValidationServiceFactory::create();
        REQUIRE(service != nullptr);
    }
    
    SECTION("Create with custom configuration") {
        ExternalValidationConfig config;
        config.enableXMLSecurity = true;
        config.enableCaching = false;
        
        auto service = ExternalValidationServiceFactory::create(config);
        REQUIRE(service != nullptr);
    }
}

TEST_CASE("ExternalValidationService provides engine information", "[application][external-validation][info]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto infoResult = service->getEngineInfo();
    REQUIRE(infoResult.isSuccess());
    
    auto info = infoResult.value();
    REQUIRE_FALSE(info.engineVersion.empty());
    REQUIRE_FALSE(info.libxml2Version.empty());
    
    // REFACTOR phase: Check external authority support based on libxml2 availability
    if (info.libxml2Version.find("not-available") != std::string::npos) {
        // libxml2 not available - external authority support should be false
        REQUIRE(info.externalAuthoritySupport == false);
    } else {
        // libxml2 available - external authority support should be true
        REQUIRE(info.externalAuthoritySupport == true);
    }
    
    REQUIRE_FALSE(info.supportedSchemaTypes.empty());
}

TEST_CASE("ExternalValidationService checks schema availability", "[application][external-validation][schema]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto availabilityResult = service->checkSchemaAvailability();
    REQUIRE(availabilityResult.isSuccess());
    
    auto availability = availabilityResult.value();
    REQUIRE_FALSE(availability.projectSchemaUrl.empty());
    REQUIRE_FALSE(availability.metaDataSchemaUrl.empty());
}

TEST_CASE("ExternalValidationService validates valid DAWProject XML", "[application][external-validation][validation]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto validationResult = service->validateProjectXML(VALID_DAWPROJECT_XML);
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    REQUIRE(report.isValid == true);
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    REQUIRE_FALSE(report.schemaPath.empty());
    REQUIRE_FALSE(report.externalSchemaSource.empty());
    
    // Performance requirement: validation should complete within 2 seconds
    REQUIRE(report.validationTime <= std::chrono::seconds(2));
    
    // Successful validation should have no errors
    REQUIRE(report.totalErrorCount == 0);
}

TEST_CASE("ExternalValidationService detects invalid DAWProject XML", "[application][external-validation][validation]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto validationResult = service->validateProjectXML(INVALID_DAWPROJECT_XML);
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    REQUIRE(report.isValid == false);
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    
    // Should detect validation errors
    REQUIRE_FALSE(report.isValid);
    REQUIRE(report.totalErrorCount > 0);
    
    // Should contain error about missing version attribute
    bool foundVersionError = false;
    for (const auto& error : report.errors) {
        if (error.message.find("version") != std::string::npos) {
            foundVersionError = true;
            break;
        }
    }
    REQUIRE(foundVersionError);
}

TEST_CASE("ExternalValidationService handles malformed XML", "[application][external-validation][validation]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto validationResult = service->validateProjectXML(MALFORMED_XML);
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    // Should detect malformed XML
    REQUIRE_FALSE(report.isValid);
    REQUIRE(report.totalErrorCount > 0);
    
    // Should contain error about unclosed tag
    bool foundParseError = false;
    for (const auto& error : report.errors) {
        if (error.message.find("Unclosed") != std::string::npos || 
            error.message.find("tag") != std::string::npos ||
            error.message.find("parse") != std::string::npos) {
            foundParseError = true;
            break;
        }
    }
    REQUIRE(foundParseError);
}

TEST_CASE("ExternalValidationService validates metadata XML", "[application][external-validation][validation]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    auto validationResult = service->validateMetaDataXML(VALID_METADATA_XML);
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    REQUIRE(report.isValid == true);
    REQUIRE(report.schemaType == SchemaType::MetaDataSchema);
    REQUIRE(report.externalAuthorityCompliant == true);
    
    // Performance requirement: metadata validation should be fast
    REQUIRE(report.validationTime <= std::chrono::seconds(1));
}

TEST_CASE("ExternalValidationService validates XML file", "[application][external-validation][file]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    // Create temporary test file
    std::filesystem::path tempFile = std::filesystem::temp_directory_path() / "test_dawproject.xml";
    {
        std::ofstream file(tempFile);
        file << VALID_DAWPROJECT_XML;
    }
    
    auto validationResult = service->validateXMLFile(tempFile, SchemaType::ProjectSchema);
    REQUIRE(validationResult.isSuccess());
    
    auto report = validationResult.value();
    REQUIRE(report.schemaType == SchemaType::ProjectSchema);
    
    // Cleanup
    std::filesystem::remove(tempFile);
}

TEST_CASE("ExternalValidationService supports batch validation", "[application][external-validation][batch]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    std::vector<ValidationRequest> requests = {
        {VALID_DAWPROJECT_XML, SchemaType::ProjectSchema, "request1"},
        {INVALID_DAWPROJECT_XML, SchemaType::ProjectSchema, "request2"},
        {VALID_METADATA_XML, SchemaType::MetaDataSchema, "request3"}
    };
    
    auto batchResult = service->batchValidate(requests);
    REQUIRE(batchResult.isSuccess());
    
    auto reports = batchResult.value();
    REQUIRE(reports.size() == requests.size());
    
    // Check individual results
    REQUIRE(reports[0].schemaType == SchemaType::ProjectSchema);
    REQUIRE(reports[1].schemaType == SchemaType::ProjectSchema);
    REQUIRE(reports[2].schemaType == SchemaType::MetaDataSchema);
}

TEST_CASE("ExternalValidationService updates configuration", "[application][external-validation][config]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    ExternalValidationConfig newConfig;
    newConfig.enableXMLSecurity = false;
    newConfig.enableCaching = true;
    
    auto updateResult = service->updateConfiguration(newConfig);
    REQUIRE(updateResult.isSuccess());
}

TEST_CASE("ExternalValidationService meets performance requirements", "[application][external-validation][performance]") {
    auto service = ExternalValidationServiceFactory::create();
    REQUIRE(service != nullptr);
    
    // Create a larger test document (simulate ~1MB)
    std::string largeXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<Project version=\"1.0\" xmlns=\"http://www.bitwig.com/dawproject\">\n";
    
    // Add many elements to simulate larger document
    for (int i = 0; i < 1000; ++i) {
        largeXML += "  <Track id=\"" + std::to_string(i) + "\" name=\"Track " + std::to_string(i) + "\">\n";
        largeXML += "    <Clips>\n";
        for (int j = 0; j < 10; ++j) {
            largeXML += "      <Clip id=\"" + std::to_string(i * 10 + j) + "\" />\n";
        }
        largeXML += "    </Clips>\n";
        largeXML += "  </Track>\n";
    }
    largeXML += "</Project>";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto validationResult = service->validateProjectXML(largeXML);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    
    // Performance requirement: should complete within 2 seconds for large files
    REQUIRE(duration <= std::chrono::seconds(2));
}