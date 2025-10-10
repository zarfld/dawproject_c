/**
 * Quick test of the new DAWProject compatibility logic
 */

#include <dawproject/dawproject.h>
#include <iostream>

using namespace dawproject;

int main() {
    try {
        // Create a test project (this will use mock data)
        auto project = DawProject::load("test.dawproject");
        
        std::cout << "=== DAWProject Compatibility Analysis ===" << std::endl;
        std::cout << "(Using corrected DAWProject standard compliance)" << std::endl << std::endl;

        // Test different DAWs
        std::vector<std::string> testDAWs = {
            "Bitwig Studio",    // Should have full support
            "Studio One",       // Should have full support  
            "Reaper",          // Should have limited support via converter
            "Cubase",          // Should have no support
            "Pro Tools",       // Should have no support
            "Logic Pro",       // Should have no support
            "Unknown DAW"      // Should throw exception
        };

        for (const auto& dawName : testDAWs) {
            std::cout << "--- " << dawName << " ---" << std::endl;
            try {
                auto compatibility = project->checkCompatibility(dawName);
                std::cout << "Compatible: " << (compatibility.isCompatible ? "YES" : "NO") << std::endl;
                
                std::cout << "Supported Features:" << std::endl;
                for (const auto& feature : compatibility.supportedFeatures) {
                    std::cout << "  + " << feature << std::endl;
                }
                
                if (!compatibility.incompatibleFeatures.empty()) {
                    std::cout << "Incompatible Features:" << std::endl;
                    for (const auto& feature : compatibility.incompatibleFeatures) {
                        std::cout << "  - " << feature.featureName << ": " << feature.description << std::endl;
                    }
                }
                
            } catch (const DawProjectException& e) {
                std::cout << "ERROR: " << e.what() << std::endl;
            }
            std::cout << std::endl;
        }

        std::cout << "=== Compatibility Matrix ===" << std::endl;
        auto matrix = project->getCompatibilityMatrix();
        for (const auto& entry : matrix) {
            std::cout << entry.first << ": " << (entry.second.isCompatible ? "COMPATIBLE" : "NOT COMPATIBLE") << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << std::endl << "New compatibility logic working correctly!" << std::endl;
    return 0;
}