---
specType: architecture
standard: "ISO/IEC/IEEE 42010:2011"
phase: "03-architecture"
version: "1.0.0"
author: "Architecture Team"
date: "2025-10-03"
status: "approved"
traceability:
    requirements:
        - "REQ-F-001"
---

# ADR-003: ZIP Library Selection (minizip)

## Status

Accepted (2025-10-03) – minizip selected for lightweight streaming archive support.

**Deciders**: Architecture Team  
**Technical Story**: Technology Stack Selection for ZIP Archive Handling

## Context

The DAW Project standard uses ZIP containers to bundle XML, audio, and metadata files. The library must:
- Read and write ZIP archives cross-platform (Windows, macOS, Linux)
- Support streaming and random access to large files
- Be compatible with open source licensing (MIT)
- Integrate cleanly with C++17 and CMake
- Have a small dependency footprint

## Decision

We will use **minizip** (from the zlib distribution) as the primary ZIP archive library.

## Rationale

### minizip Advantages
- **Lightweight**: Minimal codebase, easy to audit and maintain
- **Cross-Platform**: Works on all target OSes
- **Streaming Support**: Can read/write large files without full extraction
- **C API**: Easy to wrap for C++ and C-style APIs
- **License**: zlib license (MIT-compatible)
- **Active Maintenance**: Used in many open source projects
- **CMake Integration**: Simple to add as submodule or external project

### Comparison with Alternatives
| Feature | minizip | libzip | 7-Zip SDK |
|---------|---------|--------|-----------|
| License | zlib ✅ | BSD-3 ✅ | LGPL/GPL ❌ |
| Streaming | Yes ✅ | Yes ✅ | Yes ✅ |
| C++ API | No ⚠️ | Yes ✅ | No ⚠️ |
| Size | Small ✅ | Medium ⚠️ | Large ❌ |
| Maintenance | Active ✅ | Active ✅ | Inactive ⚠️ |
| CMake | Good ✅ | Good ✅ | Poor ❌ |

#### ❌ **libzip**
- Larger dependency, more complex API
- BSD license is compatible, but more features than needed
- C++ wrappers available, but not as lightweight

#### ❌ **7-Zip SDK**
- Complex integration, not designed for embedding
- LGPL/GPL license not compatible with MIT
- Overkill for DAW Project use case

## Implementation Strategy
- Add minizip as a submodule or via vcpkg/Conan
- Wrap minizip C API in a thin C++ layer for RAII and error handling
- Expose streaming and random access APIs to core engine
- Ensure cross-platform builds and CI coverage

## Consequences

### Positive
- ✅ Minimal dependency footprint
- ✅ Easy integration with C++ and C APIs
- ✅ Meets all performance and compatibility requirements
- ✅ License compatible with MIT

### Negative
- ⚠️ No native C++ API (must wrap in C++)
- ⚠️ Fewer advanced features than libzip (not needed for this project)

## Risk Assessment
| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| API changes upstream | Low | Low | Pin version, test upgrades |
| Streaming bugs | Medium | Low | Automated tests, fuzzing |
| Build issues | Medium | Low | CI on all platforms |

## Compliance with Requirements
| Requirement | minizip Support |
|-------------|-----------------|
| **FR-FILE-001 (Load DAW Project File)** | ✅ ZIP extraction and reading |
| **FR-FILE-003 (Streaming Access)** | ✅ Streaming read/write |
| **NFR-COMPAT-001 (Cross-Platform)** | ✅ All platforms supported |
| **NFR-PERF-002 (Memory Usage)** | ✅ Streaming, no full extraction |

## References
- **minizip**: https://github.com/zlib-ng/minizip-ng
- **libzip**: https://libzip.org/
- **DAW Project Standard**: https://github.com/bitwig/dawproject

---

*This ADR documents the ZIP library decision for the DAW Project C++ implementation.*