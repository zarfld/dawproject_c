#pragma once

#include "validation_models.h"
#include "../core/result.h"
#include <memory>

namespace dawproject {
namespace external {

/**
 * @brief External XSD validation engine using authoritative schemas only
 * 
 * Provides XSD validation against external DAWProject schemas using libxml2.
 * Ensures external authority compliance by using only external schemas.
 * 
 * @threadsafety Thread-safe for concurrent validation operations
 * @performance Validation: <2s for typical project files (<1MB XML)
 * @reliability Zero false positives - only authoritative XSD validation
 */
class IExternalValidationEngine {
public:
    virtual ~IExternalValidationEngine() = default;

    /**
     * @brief Validate project XML against external Project.xsd
     * @param xmlContent XML content to validate (UTF-8 encoded)
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     * @performance <2s for files <1MB
     */
    virtual Result<ValidationReport> validateProjectXML(const std::string& xmlContent) = 0;

    /**
     * @brief Validate metadata XML against external MetaData.xsd
     * @param xmlContent XML content to validate (UTF-8 encoded)
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     * @performance <1s for typical metadata
     */
    virtual Result<ValidationReport> validateMetaDataXML(const std::string& xmlContent) = 0;

    /**
     * @brief Validate XML file against specified external schema type
     * @param xmlFilePath Path to XML file to validate
     * @param schemaType Type of schema to validate against
     * @return Result<ValidationReport> Detailed validation results
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<ValidationReport> validateXMLFile(
        const std::filesystem::path& xmlFilePath,
        SchemaType schemaType) = 0;

    /**
     * @brief Perform batch validation of multiple XML contents
     * @param validationRequests Vector of validation requests
     * @return Result<std::vector<ValidationReport>> Results for each request
     * @throws None (uses Result<T> pattern)
     * @performance Parallel processing for improved throughput
     */
    virtual Result<std::vector<ValidationReport>> batchValidate(
        const std::vector<ValidationRequest>& validationRequests) = 0;

    /**
     * @brief Check availability of external schemas
     * @return Result<SchemaAvailabilityInfo> Schema availability status
     * @throws None (uses Result<T> pattern)
     * @network May perform network requests to check schema availability
     */
    virtual Result<SchemaAvailabilityInfo> checkSchemaAvailability() = 0;

    /**
     * @brief Get information about the validation engine
     * @return Result<ValidationEngineInfo> Engine information and capabilities
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<ValidationEngineInfo> getEngineInfo() = 0;

    /**
     * @brief Update validation configuration
     * @param config New validation configuration
     * @return Result<void> Success or error information
     * @throws None (uses Result<T> pattern)
     */
    virtual Result<void> updateConfiguration(const ExternalValidationConfig& config) = 0;
};

/**
 * @brief Factory for creating ExternalValidationEngine instances
 */
class ExternalValidationEngineFactory {
public:
    /**
     * @brief Create ExternalValidationEngine with default configuration
     * @return std::unique_ptr<IExternalValidationEngine> Validation engine instance
     * @throws std::runtime_error If engine cannot be created
     */
    static std::unique_ptr<IExternalValidationEngine> create();

    /**
     * @brief Create ExternalValidationEngine with custom configuration
     * @param config Validation configuration
     * @return std::unique_ptr<IExternalValidationEngine> Validation engine instance
     * @throws std::runtime_error If engine cannot be created
     */
    static std::unique_ptr<IExternalValidationEngine> create(const ExternalValidationConfig& config);
};

} // namespace external
} // namespace dawproject