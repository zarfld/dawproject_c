// TEST-UNIT-SMOKE-001
// Trace: REQ-F-001, REQ-F-002, REQ-F-003, REQ-NF-P-001
// Purpose: Minimal smoke test to ensure some executable lines are covered
// so coverage tooling does not report 0% when other tests are filtered.

#include <catch2/catch_test_macros.hpp>
#include <dawproject/platform/factory.h>
#include <dawproject/data/data_access_factory.h>

using namespace dawproject::platform;
using namespace dawproject::data;

TEST_CASE("Smoke: create core components", "[smoke][coverage]") {
    auto &factory = PlatformFactory::getInstance();
    auto mem = factory.createMemoryManager();
    REQUIRE(mem != nullptr);
    void* ptr = mem->allocate(32);
    REQUIRE(ptr != nullptr);
    mem->deallocate(ptr);

    auto fs = factory.createFileSystem();
    REQUIRE(fs != nullptr);
    // Basic call that should succeed or throw deterministically
    REQUIRE_NOTHROW(fs->getTempDirectory());

    auto engine = DataAccessFactory::createDataAccessEngine();
    REQUIRE(engine != nullptr);
    auto reader = engine->createReader("nonexistent.dawproject");
    REQUIRE(reader != nullptr); // Reader construction succeeds; open() failure acceptable for nonexistent file.
}
