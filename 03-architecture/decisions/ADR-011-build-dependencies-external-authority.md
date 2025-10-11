# ADR-011: Build Dependencies for External Authority Compliance

**Status**: PROPOSED  
**Date**: 2025-01-09  
**Deciders**: Development Team, Architecture Review Board  
**Technical Story**: [External Authority Compliance] Update build system to support external schema validation and management

## Context

External authority compliance analysis identified critical dependency requirements for standards-compliant operation:

**Required Capabilities**:
- External XSD schema validation against Project.xsd and MetaData.xsd
- HTTP client for downloading external schemas and test files
- Fast XML parsing for performance requirements
- ZIP archive processing for .dawproject files

**Previous State**: 
- Only pugixml and basic threading dependencies
- No external schema validation capability
- No network operations for external authority integration

**Compliance Gap**: 
- Cannot validate against external authoritative schemas
- Cannot download official test files for verification
- Limited to custom validation logic (non-compliant)

## Decision

We will add **four critical dependency categories** to support external authority compliance:

### 1. External XSD Validation: libxml2

```cmake
# External XSD validation support (libxml2) - CRITICAL for external authority compliance
if(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
    else()
        find_package(LibXml2 REQUIRED)
    endif()
endif()
```

**Rationale**: 
- Industry standard for XSD schema validation
- Required for validating against external Project.xsd and MetaData.xsd
- No alternative provides equivalent XSD validation capability

### 2. External Schema Management: libcurl

```cmake
# HTTP client for external schema management (libcurl)
if(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT)
    find_package(CURL REQUIRED)
endif()
```

**Rationale**:
- Robust, cross-platform HTTP client library
- Required for downloading external schemas from DAWProject repository
- Supports caching, retry logic, and error handling

### 3. Fast XML Parsing: pugixml (Enhanced Integration)

```cmake
# pugixml for fast XML parsing (header-only preferred)
find_path(PUGIXML_INCLUDE_DIR pugixml.hpp
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pugixml/src
    PATHS /usr/include /usr/local/include
)
```

**Rationale**:
- Maintains existing fast parsing capability
- Works alongside libxml2 for dual parsing strategy
- Header-only for easy integration

### 4. ZIP Archive Processing: libzip

```cmake
# ZIP library for DAWProject archive handling
if(PkgConfig_FOUND)
    pkg_check_modules(LIBZIP libzip)
endif()
```

**Rationale**:
- DAWProject files are ZIP archives with XML content
- Cross-platform archive handling capability
- Production-ready ZIP implementation

## Build Configuration Strategy

### Conditional Compilation

```cmake
option(DAWPROJECT_ENABLE_EXTERNAL_VALIDATION "Enable external XSD validation with libxml2" ON)
option(DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT "Enable external schema download with libcurl" ON)
```

**Benefits**:
- Allow builds without network dependencies for specific use cases
- Enable gradual migration to external authority compliance
- Provide fallback options for constrained environments

### Preprocessor Definitions

```cmake
# Conditional compilation based on available dependencies
target_compile_definitions(dawproject PRIVATE 
    DAWPROJECT_HAS_LIBXML2     # Enable XSD validation code paths
    DAWPROJECT_HAS_CURL        # Enable external schema management
    DAWPROJECT_HAS_PUGIXML     # Enable fast XML parsing
    DAWPROJECT_HAS_LIBZIP      # Enable ZIP archive support
)
```

### Cross-Platform Considerations

#### Windows (MSVC/MinGW)
```cmake
if(WIN32)
    # Windows-specific libcurl configuration
    set(CURL_USE_STATIC_LIBS ON)
    
    # libxml2 Windows configuration
    if(MSVC)
        set(LIBXML2_DEFINITIONS -DLIBXML_STATIC)
    endif()
endif()
```

#### Linux (GCC/Clang)
```cmake
if(UNIX AND NOT APPLE)
    # Linux package manager integration
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
    pkg_check_modules(LIBZIP REQUIRED libzip)
endif()
```

#### macOS (Clang/AppleClang)
```cmake
if(APPLE)
    # Homebrew/MacPorts integration
    find_package(CURL REQUIRED)
    find_package(LibXml2 REQUIRED)
endif()
```

## Alternatives Considered

### ❌ **Embedded Schema Copies**
- **Pros**: No network dependency, faster builds
- **Cons**: Violates external authority principle, manual update burden
- **Verdict**: Rejected - fails compliance requirements

### ❌ **Custom HTTP Implementation**
- **Pros**: Reduced dependencies, tailored functionality
- **Cons**: Security risks, maintenance burden, platform complexity
- **Verdict**: Rejected - libcurl is industry standard

### ❌ **Alternative XML Libraries**
- **Pros**: Fewer dependencies
- **Cons**: No mature XSD validation alternatives to libxml2
- **Verdict**: Rejected - libxml2 required for XSD compliance

### ❌ **Single XML Library Strategy**
- **Pros**: Simpler dependency management
- **Cons**: Performance vs compliance trade-off
- **Verdict**: Rejected - dual strategy provides best of both worlds

## Consequences

### Positive

✅ **External Authority Compliance**: Enables validation against official schemas  
✅ **Standards Compliance**: XSD validation ensures DAWProject specification adherence  
✅ **Network Capability**: Can download and cache external resources  
✅ **Performance Optimization**: Dual XML strategy (fast parsing + XSD validation)  
✅ **Cross-Platform Support**: All dependencies available on target platforms  
✅ **Gradual Adoption**: Optional features allow phased migration  
✅ **Production Ready**: All dependencies are mature, well-maintained libraries

### Negative

⚠️ **Increased Build Complexity**: More dependencies to manage and configure  
**Mitigation**: CMake automation with clear error messages for missing dependencies  

⚠️ **Larger Binary Size**: Additional libraries increase executable size  
**Mitigation**: Static linking options and dependency optimization  

⚠️ **Build Time Impact**: More dependencies may slow compilation  
**Mitigation**: Header-only libraries where possible, parallel builds  

⚠️ **Platform-Specific Issues**: Different package managers, library locations  
**Mitigation**: Comprehensive CMake find modules with fallback strategies

### Risk Assessment

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| **Dependency Unavailable** | High | Low | Clear documentation, package manager guides |
| **Version Conflicts** | Medium | Medium | Minimum version requirements, compatibility testing |
| **Build System Complexity** | Medium | Medium | Automated dependency detection, clear error messages |
| **Cross-Platform Issues** | High | Medium | Extensive CI testing on all target platforms |
| **License Compatibility** | Low | Low | All dependencies use compatible licenses |

## Implementation Plan

### Phase 1: Core Dependencies
- [ ] Update CMakeLists.txt with libxml2 and libcurl detection
- [ ] Add conditional compilation flags
- [ ] Test basic dependency detection on all platforms

### Phase 2: Integration Testing
- [ ] Verify cross-platform builds with all dependencies
- [ ] Test optional dependency scenarios
- [ ] Validate library linking and symbol resolution

### Phase 3: CI/CD Integration
- [ ] Update GitHub Actions with dependency installation
- [ ] Add dependency caching for build performance
- [ ] Validate builds on Windows, Linux, macOS

### Phase 4: Documentation
- [ ] Update build instructions with dependency requirements
- [ ] Create platform-specific installation guides
- [ ] Document troubleshooting for common dependency issues

## Dependency Requirements

### Minimum Versions

| Dependency | Minimum Version | Rationale |
|------------|----------------|-----------|
| **libxml2** | 2.9.0+ | XSD 1.1 support, security fixes |
| **libcurl** | 7.60.0+ | Modern TLS support, HTTP/2 capability |
| **pugixml** | 1.11+ | C++17 compatibility, performance improvements |
| **libzip** | 1.5.0+ | CMake support, modern API |

### Optional Features

```cmake
# Enable/disable specific external authority features
DAWPROJECT_ENABLE_EXTERNAL_VALIDATION     # XSD validation (requires libxml2)
DAWPROJECT_ENABLE_EXTERNAL_SCHEMA_MANAGEMENT  # Schema download (requires libcurl)
DAWPROJECT_ENABLE_PUGIXML                 # Fast XML parsing (requires pugixml)
DAWPROJECT_ENABLE_LIBZIP                  # ZIP archive support (requires libzip)
```

## Verification

### Build Verification Script

```bash
#!/bin/bash
# verify_dependencies.sh - Validate all required dependencies

echo "Verifying DAWProject C++ Dependencies..."

# Check CMake version
cmake --version || { echo "ERROR: CMake not found"; exit 1; }

# Check compiler
${CXX:-g++} --version || { echo "ERROR: C++ compiler not found"; exit 1; }

# Check pkg-config (optional but recommended)
pkg-config --version && echo "✓ pkg-config available"

# Check libxml2
pkg-config --exists libxml-2.0 && echo "✓ libxml2 found" || echo "⚠ libxml2 missing"

# Check libcurl
pkg-config --exists libcurl && echo "✓ libcurl found" || echo "⚠ libcurl missing"

# Check libzip
pkg-config --exists libzip && echo "✓ libzip found" || echo "⚠ libzip missing"

echo "Dependency check complete"
```

## References

- **CMake Documentation**: <https://cmake.org/documentation/>
- **libxml2 Documentation**: <http://xmlsoft.org/html/>
- **libcurl Documentation**: <https://curl.se/libcurl/>
- **pugixml Documentation**: <https://pugixml.org/>
- **libzip Documentation**: <https://libzip.org/>

---

*This ADR establishes the build dependency strategy ensuring external authority compliance while maintaining cross-platform compatibility and build system robustness.*