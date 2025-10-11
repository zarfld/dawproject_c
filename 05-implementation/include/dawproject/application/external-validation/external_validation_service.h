#pragma once

#include "../../domain/validation/validation_models.h"
#include "../../core/result.h"
#include <memory>

namespace dawproject {
namespace application {
namespace external_validation {

// Import domain types
using namespace dawproject::domain::validation;

/**
 * @brief External validation service interface (Application Layer)
 * 
 * Defines the use cases for external XSD validation against authoritative schemas.
 * This is the application service layer that orchestrates domain validation logic.
 * 
 * @threadsafety Thread-safe for concurrent validation operations
 * @performance Validation: <2s for typical project files (<1MB XML)
 * @reliability Zero false positives - only authoritative XSD validation
 */
class IExternalValidationService {
public:
    virtual ~IExternalValidationService() = default;

    /**
     * @brief Use Case: Validate project XML against external Project.xsd
     * @param xmlContent XML content to validate (UTF-8 encoded)
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     * @performance <2s for files <1MB
     */
    virtual Result<ValidationReport> validateProjectXML(const std::string& xmlContent) = 0;

    /**
     * @brief Use Case: Validate metadata XML against external MetaData.xsd
     * @param xmlContent XML content to validate (UTF-8 encoded)
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     * @performance <1s for typical metadata
     */
    virtual Result<ValidationReport> validateMetaDataXML(const std::string& xmlContent) = 0;

    /**
     * @brief Use Case: Validate XML file against specified external schema type
     * @param xmlFilePath Path to XML file to validate
     * @param schemaType Type of schema to validate against
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<ValidationReport> validateXMLFile(
        const std::filesystem::path& xmlFilePath,
        SchemaType schemaType) = 0;

    /**
     * @brief Use Case: Perform batch validation of multiple XML contents
     * @param validationRequests Vector of validation requests
     * @return Result<std::vector<ValidationReport>> Results for each request
     * @throws None (uses Result<T> pattern)
     * @performance Parallel processing for improved throughput
     */
    virtual Result<std::vector<ValidationReport>> batchValidate(
        const std::vector<ValidationRequest>& validationRequests) = 0;

    /**
     * @brief Use Case: Check availability of external schemas
     * @return Result<SchemaAvailabilityInfo> Schema availability status
     * @throws None (uses Result<T> pattern)
     * @network May perform network requests to check schema availability
     */
    virtual Result<SchemaAvailabilityInfo> checkSchemaAvailability() = 0;

    /**
     * @brief Use Case: Get information about the validation engine
     * @return Result<ValidationEngineInfo> Engine information and capabilities
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<ValidationEngineInfo> getEngineInfo() = 0;

    /**
     * @brief Use Case: Update validation configuration
     * @param config New validation configuration
     * @return Result<void> Success or error information
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<void> updateConfiguration(const ExternalValidationConfig& config) = 0;
};

/**
 * @brief Factory for creating ExternalValidationService instances (Application Layer)
 */
class ExternalValidationServiceFactory {
public:
    /**
     * @brief Create ExternalValidationService with default configuration
     * @return std::unique_ptr<IExternalValidationService> Validation service instance
     * @throws std::runtime_error If service cannot be created
     */
    static std::unique_ptr<IExternalValidationService> create();

    /**
     * @brief Create ExternalValidationService with custom configuration
     * @param config Validation configuration
     * @return std::unique_ptr<IExternalValidationService> Validation service instance
     * @throws std::runtime_error If service cannot be created
     */
    static std::unique_ptr<IExternalValidationService> create(const ExternalValidationConfig& config);
};

} // namespace external_validation
} // namespace application
} // namespace dawproject