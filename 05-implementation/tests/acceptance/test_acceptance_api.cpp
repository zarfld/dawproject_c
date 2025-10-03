#include <catch2/catch_test_macros.hpp>
#include <string>

// High-level acceptance test framing REQ-F-015 (dual API assumption) & REQ-NF-008 (testability)
// NOTE: Will be refactored once real public API headers are in place.
namespace dawproject {
    struct CppFacade { int version() const { return 1; } }; // REQ-F-015 placeholder
    extern "C" int dawproject_c_version() { return 1; } // REQ-F-015 placeholder C-style symbol
}

TEST_CASE("Public API version consistency (REQ-F-015, REQ-NF-008)", "[acceptance][api]") {
    dawproject::CppFacade facade;
    REQUIRE(facade.version() == 1);
    REQUIRE(dawproject::dawproject_c_version() == 1);
}
