#pragma once

#include <dawproject/data/data_access_engine.h>
#include <filesystem>
#include <memory>

namespace dawproject::data {

    /**
     * @brief XML Processor Implementation using pugixml
     * 
     * Provides XML document loading, parsing, saving, and validation capabilities.
     * This implementation uses pugixml for robust XML processing and follows
     * the IXMLProcessor interface contract with enhanced error handling.
     * 
     * @note Implements SOLID principles with improved const-correctness
     * @note Enhanced input validation and exception safety
     */
    class XMLProcessorImpl : public IXMLProcessor {
    public:
        XMLProcessorImpl() = default;
        virtual ~XMLProcessorImpl() = default;

        // IXMLProcessor interface implementation with const-correctness
        Result<XMLDocument> loadDocument(const std::filesystem::path& path) override;
        Result<XMLDocument> parseDocument(const std::string& xmlContent) override;
        WriteResult saveDocument(const XMLDocument& doc, const std::filesystem::path& path) override;
        ValidationResult validateXML(const std::filesystem::path& path) override;
        ValidationResult validateAgainstSchema(const XMLDocument& doc, const std::filesystem::path& schemaPath) override;

        // Copy/move semantics
        XMLProcessorImpl(const XMLProcessorImpl&) = delete;
        XMLProcessorImpl& operator=(const XMLProcessorImpl&) = delete;
        XMLProcessorImpl(XMLProcessorImpl&&) = default;
        XMLProcessorImpl& operator=(XMLProcessorImpl&&) = default;
    };

} // namespace dawproject::data