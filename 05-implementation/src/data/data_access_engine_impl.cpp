#include <dawproject/data/data_access_engine.h>
#include "xml_processor_impl.h"
#include "zip_processor_impl.h"
#include "data_access_engine_impl.h"
#include <algorithm>
#include <memory>
#include <fstream>
#include <pugixml.hpp>

namespace dawproject::data {

    // Forward declarations  
    class DataAccessEngineImpl;

    /**
     * @brief Minimal ZIP Processor Implementation for GREEN phase
     * Using file system simulation instead of actual ZIP operations
     */

    




    // Implementation details will be in separate methods below...

    // ProjectInfo validation
    bool ProjectInfo::isValid() const {
        return !title.empty() && tempo > 0.0;
    }

    std::vector<std::string> ProjectInfo::getValidationErrors() const {
        std::vector<std::string> errors;
        
        if (title.empty()) {
            errors.push_back("Project title cannot be empty");
        }
        
        if (tempo <= 0.0) {
            errors.push_back("Project tempo must be greater than 0");
        }
        
        if (timeSignature.empty()) {
            errors.push_back("Time signature cannot be empty");
        }
        
        return errors;
    }

    // TrackInfo validation
    bool TrackInfo::isValid() const {
        return !id.empty() && !name.empty() && volume >= 0.0 && volume <= 1.0;
    }

    // ClipInfo validation
    bool ClipInfo::isValid() const {
        return !id.empty() && !name.empty() && !trackId.empty() && 
               duration > 0.0 && startTime >= 0.0;
    }

    // ValidationResult operations
    void ValidationResult::addError(const std::string& message) {
        errors.push_back(message);
        isValid = false;
    }

    void ValidationResult::addWarning(const std::string& message) {
        warnings.push_back(message);
    }

    void ValidationResult::merge(const ValidationResult& other) {
        errors.insert(errors.end(), other.errors.begin(), other.errors.end());
        warnings.insert(warnings.end(), other.warnings.begin(), other.warnings.end());
        if (!other.isValid) {
            isValid = false;
        }
    }

    // XMLDocument helper methods
    XMLDocument* XMLDocument::findChild(const std::string& name) {
        auto it = std::find_if(children.begin(), children.end(),
            [&name](const XMLDocument& child) {
                return child.rootElementName == name;
            });
        return (it != children.end()) ? &(*it) : nullptr;
    }

    const XMLDocument* XMLDocument::findChild(const std::string& name) const {
        auto it = std::find_if(children.begin(), children.end(),
            [&name](const XMLDocument& child) {
                return child.rootElementName == name;
            });
        return (it != children.end()) ? &(*it) : nullptr;
    }

    std::vector<XMLDocument*> XMLDocument::findChildren(const std::string& name) {
        std::vector<XMLDocument*> result;
        for (auto& child : children) {
            if (child.rootElementName == name) {
                result.push_back(&child);
            }
        }
        return result;
    }

    std::string XMLDocument::getAttribute(const std::string& name, 
                                         const std::string& defaultValue) const {
        auto it = attributes.find(name);
        return (it != attributes.end()) ? it->second : defaultValue;
    }

    void XMLDocument::setAttribute(const std::string& name, const std::string& value) {
        attributes[name] = value;
    }







    // XMLProcessorImpl implementation


    // ZIPProcessorImpl implementation  


} // namespace dawproject::data

// Note: Factory functions moved to data_access_engine_extracted.cpp during REFACTOR phase