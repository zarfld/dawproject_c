#include "libxml2_validator.h"

#ifdef DAWPROJECT_HAS_LIBXML2
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlstring.h>
#endif

#include <chrono>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sstream>

namespace dawproject::external {

struct LibXML2Validator::Impl {
#ifdef DAWPROJECT_HAS_LIBXML2
    xmlSchemaPtr schema = nullptr;
    xmlSchemaValidCtxtPtr validationContext = nullptr;
#endif
    std::vector<ValidationError> currentErrors;
    std::string schemaVersion;
    bool initialized = false;

    Impl() {
#ifdef DAWPROJECT_HAS_LIBXML2
        // Initialize libxml2 if not already done
        xmlInitParser();
        initialized = true;
#else
        initialized = false; // libxml2 not available
#endif
    }

    ~Impl() {
        cleanup();
#ifdef DAWPROJECT_HAS_LIBXML2
        if (initialized) {
            xmlCleanupParser();
        }
#endif
    }

    void cleanup() {
#ifdef DAWPROJECT_HAS_LIBXML2
        if (validationContext) {
            xmlSchemaFreeValidCtxt(validationContext);
            validationContext = nullptr;
        }
        if (schema) {
            xmlSchemaFree(schema);
            schema = nullptr;
        }
#endif
    }
};

#ifdef DAWPROJECT_HAS_LIBXML2
// Static callback functions for error handling
void LibXML2Validator::errorCallback(void* ctx, const char* msg, ...) {
    auto* validator = static_cast<LibXML2Validator*>(ctx);
    if (!validator || !msg) return;

    va_list args;
    va_start(args, msg);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    
    ValidationError error;
    error.line = -1;
    error.column = -1;
    error.message = buffer;
    error.severity = "error";
    
    validator->pImpl->currentErrors.push_back(error);
    
    va_end(args);
}

void LibXML2Validator::warningCallback(void* ctx, const char* msg, ...) {
    auto* validator = static_cast<LibXML2Validator*>(ctx);
    if (!validator || !msg) return;

    va_list args;
    va_start(args, msg);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    
    ValidationError error;
    error.line = -1;
    error.column = -1;
    error.message = buffer;
    error.severity = "warning";
    
    validator->pImpl->currentErrors.push_back(error);
    
    va_end(args);
}

void LibXML2Validator::structuredErrorCallback(void* userData, xmlErrorPtr error) {
    auto* validator = static_cast<LibXML2Validator*>(userData);
    if (!validator || !error) return;

    ValidationError valError;
    valError.line = error->line;
    valError.column = error->int2;
    valError.message = error->message ? error->message : "Unknown error";
    
    switch (error->level) {
        case XML_ERR_WARNING:
            valError.severity = "warning";
            break;
        case XML_ERR_ERROR:
            valError.severity = "error";
            break;
        case XML_ERR_FATAL:
            valError.severity = "fatal";
            break;
        default:
            valError.severity = "unknown";
            break;
    }
    
    validator->pImpl->currentErrors.push_back(valError);
}
#endif // DAWPROJECT_HAS_LIBXML2

LibXML2Validator::LibXML2Validator() : pImpl(std::make_unique<Impl>()) {}

LibXML2Validator::~LibXML2Validator() = default;

LibXML2Validator::LibXML2Validator(LibXML2Validator&& other) noexcept 
    : pImpl(std::move(other.pImpl)) {}

LibXML2Validator& LibXML2Validator::operator=(LibXML2Validator&& other) noexcept {
    if (this != &other) {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

bool LibXML2Validator::loadSchema(const std::string& schemaContent) {
    pImpl->cleanup();
    pImpl->currentErrors.clear();

#ifdef DAWPROJECT_HAS_LIBXML2
    // Parse the schema from string
    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewMemParserCtxt(
        schemaContent.c_str(), 
        static_cast<int>(schemaContent.length())
    );
    
    if (!parserCtxt) {
        return false;
    }

    // Set error handlers
    xmlSchemaSetParserErrors(parserCtxt, errorCallback, warningCallback, this);

    // Parse the schema
    pImpl->schema = xmlSchemaParse(parserCtxt);
    xmlSchemaFreeParserCtxt(parserCtxt);

    if (!pImpl->schema) {
        return false;
    }

    // Create validation context
    pImpl->validationContext = xmlSchemaNewValidCtxt(pImpl->schema);
    if (!pImpl->validationContext) {
        xmlSchemaFree(pImpl->schema);
        pImpl->schema = nullptr;
        return false;
    }

    // Set validation error handlers
    xmlSchemaSetValidErrors(pImpl->validationContext, errorCallback, warningCallback, this);
    xmlSetStructuredErrorFunc(this, structuredErrorCallback);

    // Extract schema version if available
    // This is a simplified approach - actual schema version extraction would be more complex
    if (schemaContent.find("version=\"1.0\"") != std::string::npos) {
        pImpl->schemaVersion = "1.0";
    } else if (schemaContent.find("DAWProject") != std::string::npos) {
        pImpl->schemaVersion = "DAWProject-1.0";
    } else {
        pImpl->schemaVersion = "Unknown";
    }

    return true;
#else
    // Fallback implementation when libxml2 is not available
    // Still parse schema version for compatibility
    if (schemaContent.find("version=\"1.0\"") != std::string::npos) {
        pImpl->schemaVersion = "1.0-fallback";
    } else if (schemaContent.find("DAWProject") != std::string::npos) {
        pImpl->schemaVersion = "DAWProject-1.0-fallback";
    } else {
        pImpl->schemaVersion = "Unknown-fallback";
    }
    return false; // Cannot actually load schema without libxml2
#endif
}

bool LibXML2Validator::loadSchemaFromFile(const std::string& schemaPath) {
    pImpl->cleanup();
    pImpl->currentErrors.clear();

#ifdef DAWPROJECT_HAS_LIBXML2
    // Parse the schema from file
    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewParserCtxt(schemaPath.c_str());
    
    if (!parserCtxt) {
        return false;
    }

    // Set error handlers
    xmlSchemaSetParserErrors(parserCtxt, errorCallback, warningCallback, this);

    // Parse the schema
    pImpl->schema = xmlSchemaParse(parserCtxt);
    xmlSchemaFreeParserCtxt(parserCtxt);

    if (!pImpl->schema) {
        return false;
    }

    // Create validation context
    pImpl->validationContext = xmlSchemaNewValidCtxt(pImpl->schema);
    if (!pImpl->validationContext) {
        xmlSchemaFree(pImpl->schema);
        pImpl->schema = nullptr;
        return false;
    }

    // Set validation error handlers
    xmlSchemaSetValidErrors(pImpl->validationContext, errorCallback, warningCallback, this);
    xmlSetStructuredErrorFunc(this, structuredErrorCallback);

    pImpl->schemaVersion = "File-based";
    return true;
#else
    // Fallback implementation when libxml2 is not available
    pImpl->schemaVersion = "File-based-fallback";
    return false; // Cannot actually load schema without libxml2
#endif
}

LibXML2Validator::ValidationResult LibXML2Validator::validateXML(const std::string& xmlContent) {
    ValidationResult result;
    result.isValid = false;
    result.validationTimeMs = 0.0;
    result.schemaVersion = pImpl->schemaVersion;

    auto startTime = std::chrono::high_resolution_clock::now();

#ifdef DAWPROJECT_HAS_LIBXML2
    if (!isSchemaLoaded()) {
        ValidationError error;
        error.line = -1;
        error.column = -1;
        error.message = "No schema loaded for validation";
        error.severity = "fatal";
        result.errors.push_back(error);
        return result;
    }

    pImpl->currentErrors.clear();

    // Parse the XML document
    xmlDocPtr doc = xmlParseMemory(xmlContent.c_str(), static_cast<int>(xmlContent.length()));
    if (!doc) {
        auto endTime = std::chrono::high_resolution_clock::now();
        result.validationTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        
        ValidationError error;
        error.line = -1;
        error.column = -1;
        error.message = "Failed to parse XML document";
        error.severity = "fatal";
        result.errors.push_back(error);
        return result;
    }

    // Validate against schema
    int validationResult = xmlSchemaValidateDoc(pImpl->validationContext, doc);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.validationTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    // Clean up XML document
    xmlFreeDoc(doc);

    // Check validation result
    result.isValid = (validationResult == 0);
    result.errors = pImpl->currentErrors;
#else
    // Fallback implementation when libxml2 is not available
    auto endTime = std::chrono::high_resolution_clock::now();
    result.validationTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    ValidationError error;
    error.line = -1;
    error.column = -1;
    error.message = "libxml2 not available - using fallback validation";
    error.severity = "warning";
    result.errors.push_back(error);
    
    // Perform basic validation as fallback (just check if it's not empty and has XML-like structure)
    result.isValid = !xmlContent.empty() && xmlContent.find('<') != std::string::npos;
#endif

    return result;
}

bool LibXML2Validator::isSchemaLoaded() const {
#ifdef DAWPROJECT_HAS_LIBXML2
    return pImpl->schema != nullptr && pImpl->validationContext != nullptr;
#else
    return false; // Cannot load schemas without libxml2
#endif
}

std::string LibXML2Validator::getSchemaVersion() const {
    return pImpl->schemaVersion;
}

std::string LibXML2Validator::getLibXML2Version() {
#ifdef DAWPROJECT_HAS_LIBXML2
    return xmlParserVersion ? xmlParserVersion : "Unknown";
#else
    return "libxml2-not-available";
#endif
}

} // namespace dawproject::external