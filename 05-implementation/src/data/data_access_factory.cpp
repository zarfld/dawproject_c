#include <dawproject/data/data_access_factory.h>
#include <memory>

// Forward declarations of implementation classes
namespace dawproject::data {
    class XMLProcessorImpl;
    class ZIPProcessorImpl; 
    class DataAccessEngineImpl;
    class ProjectReaderImpl;
    class ProjectWriterImpl;
}

// External factory functions implemented in data_access_engine_impl.cpp
extern std::unique_ptr<dawproject::data::IXMLProcessor> createXMLProcessorImpl();
extern std::unique_ptr<dawproject::data::IZIPProcessor> createZIPProcessorImpl(); 
extern std::unique_ptr<dawproject::data::IDataAccessEngine> createDataAccessEngineImpl(
    std::unique_ptr<dawproject::data::IXMLProcessor> xmlProcessor,
    std::unique_ptr<dawproject::data::IZIPProcessor> zipProcessor);

namespace dawproject::data {

    std::unique_ptr<IDataAccessEngine> DataAccessFactory::createDataAccessEngine() {
        // GREEN phase: return working implementations via external functions
        auto xmlProcessor = createXMLProcessorImpl();
        auto zipProcessor = createZIPProcessorImpl();
        return createDataAccessEngineImpl(std::move(xmlProcessor), std::move(zipProcessor));
    }

    std::unique_ptr<IXMLProcessor> DataAccessFactory::createXMLProcessor() {
        // GREEN phase: return working XML processor
        return createXMLProcessorImpl();
    }

    std::unique_ptr<IZIPProcessor> DataAccessFactory::createZIPProcessor() {
        // GREEN phase: return working ZIP processor
        return createZIPProcessorImpl();
    }

} // namespace dawproject::data