#pragma once

#ifdef DAWPROJECT_HAS_LIBXML2
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#endif

#include <memory>
#include <string>
#include <vector>

namespace dawproject {
namespace infrastructure {
namespace xml_validation {

/**
 * @brief LibXML2 wrapper for XSD schema validation (Infrastructure Layer)
 * 
 * This class provides a RAII wrapper around libxml2 XSD validation functionality,
 * ensuring proper cleanup of resources and error handling according to TDD requirements.
 * This is part of the infrastructure layer - external technology integration.
 */
class LibXML2Validator {
public:
    struct ValidationError {
        int line;
        int column;
        std::string message;
        std::string severity; // "error", "warning", "fatal"
    };

    struct ValidationResult {
        bool isValid;
        std::vector<ValidationError> errors;
        double validationTimeMs;
        std::string schemaVersion;
    };

    /**
     * @brief Construct a new LibXML2 Validator
     * Initializes libxml2 if not already initialized
     */
    LibXML2Validator();
    
    /**
     * @brief Destroy the LibXML2 Validator
     * Cleans up any loaded schema and performs proper libxml2 cleanup
     */
    ~LibXML2Validator();

    // Non-copyable, but moveable
    LibXML2Validator(const LibXML2Validator&) = delete;
    LibXML2Validator& operator=(const LibXML2Validator&) = delete;
    LibXML2Validator(LibXML2Validator&&) noexcept;
    LibXML2Validator& operator=(LibXML2Validator&&) noexcept;

    /**
     * @brief Load XSD schema from string content
     * @param schemaContent The XSD schema content
     * @return true if schema was loaded successfully, false otherwise
     */
    bool loadSchema(const std::string& schemaContent);

    /**
     * @brief Load XSD schema from file path
     * @param schemaPath Path to the XSD schema file
     * @return true if schema was loaded successfully, false otherwise
     */
    bool loadSchemaFromFile(const std::string& schemaPath);

    /**
     * @brief Validate XML content against loaded schema
     * @param xmlContent The XML content to validate
     * @return ValidationResult with validation status and any errors
     */
    ValidationResult validateXML(const std::string& xmlContent);

    /**
     * @brief Check if a schema is currently loaded
     * @return true if schema is loaded and ready for validation
     */
    bool isSchemaLoaded() const;

    /**
     * @brief Get the version string of the loaded schema (if available)
     * @return Schema version or empty string if not available
     */
    std::string getSchemaVersion() const;

    /**
     * @brief Get libxml2 version information
     * @return libxml2 version string
     */
    static std::string getLibXML2Version();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    // Static error handling callbacks for libxml2
    static void errorCallback(void* ctx, const char* msg, ...);
    static void warningCallback(void* ctx, const char* msg, ...);
#ifdef DAWPROJECT_HAS_LIBXML2
    static void structuredErrorCallback(void* userData, xmlErrorPtr error);
#endif
};

} // namespace xml_validation
} // namespace infrastructure
} // namespace dawproject