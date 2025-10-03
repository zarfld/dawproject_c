#include <iostream>
#include <dawproject/data/data_access_factory.h>

int main() {
    try {
        std::cout << "Creating Data Access Engine..." << std::endl;
        auto engine = dawproject::data::DataAccessFactory::createDataAccessEngine();
        if (engine) {
            std::cout << "SUCCESS: Data Access Engine created!" << std::endl;
        } else {
            std::cout << "FAILED: Data Access Engine is null!" << std::endl;
            return 1;
        }

        std::cout << "Creating XML Processor..." << std::endl;
        auto xmlProcessor = dawproject::data::DataAccessFactory::createXMLProcessor();
        if (xmlProcessor) {
            std::cout << "SUCCESS: XML Processor created!" << std::endl;
        } else {
            std::cout << "FAILED: XML Processor is null!" << std::endl;
            return 1;
        }

        std::cout << "Creating ZIP Processor..." << std::endl;
        auto zipProcessor = dawproject::data::DataAccessFactory::createZIPProcessor();
        if (zipProcessor) {
            std::cout << "SUCCESS: ZIP Processor created!" << std::endl;
        } else {
            std::cout << "FAILED: ZIP Processor is null!" << std::endl;
            return 1;
        }

        std::cout << "All basic factory tests PASSED! GREEN phase is working!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}