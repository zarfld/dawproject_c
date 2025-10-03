#pragma once

#include "data_access_engine.h"
#include <memory>

namespace dawproject::data {

    // Factory for creating data access engine instances
    class DataAccessFactory {
    public:
        // Create the main data access engine
        static std::unique_ptr<IDataAccessEngine> createDataAccessEngine();
        
        // Create XML processor
        static std::unique_ptr<IXMLProcessor> createXMLProcessor();
        
        // Create ZIP processor
        static std::unique_ptr<IZIPProcessor> createZIPProcessor();
    };

} // namespace dawproject::data