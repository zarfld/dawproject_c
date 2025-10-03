# ADR-008: Build System Configuration (CMake)

**Status**: Accepted  
**Date**: October 3, 2025  
**Deciders**: Architecture Team  
**Technical Story**: Build and Dependency Management for DAW Project C++ Library

---

## Context

The library must:
- Build on Windows, macOS, and Linux
- Support static and shared library builds
- Integrate with modern IDEs (VS Code, CLion, Visual Studio)
- Manage dependencies (pugixml, minizip, Catch2)
- Enable CI/CD and package manager distribution
- Be easy for open source contributors to build

## Decision

We will use **CMake (3.20+)** as the build system.

## Rationale
- **Cross-Platform**: CMake is the de facto standard for C++
- **IDE Integration**: Works with all major IDEs
- **Dependency Management**: Integrates with vcpkg, Conan
- **Flexible**: Supports custom build types, options
- **Community Support**: Well-documented, widely used

### Alternatives Considered
- **Bazel**: Powerful but less common in C++ ecosystem
- **Meson**: Simpler syntax, but less mature for C++
- **Autotools**: Legacy, not cross-platform friendly

## Implementation Strategy
- Organize source into `src/`, `include/`, `tests/`, `examples/`
- Use `target_link_libraries` for dependencies
- Provide options for static/shared builds
- Add CPack config for packaging
- Integrate with vcpkg/Conan for dependencies
- Add CI scripts for Windows, macOS, Linux

## Consequences

### Positive
- ✅ Easy onboarding for contributors
- ✅ Reliable cross-platform builds
- ✅ Simple dependency management
- ✅ Ready for CI/CD and package managers

### Negative
- ⚠️ Learning curve for CMake newcomers
- ⚠️ Must maintain CMake scripts as project evolves

## Risk Assessment
| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Build failures | High | Low | CI on all platforms |
| Dependency drift | Medium | Low | Pin versions, update regularly |
| CMake bugs | Low | Low | Use stable releases, community support |

## Compliance with Requirements
| Requirement | Build System Support |
|-------------|---------------------|
| **NFR-COMPAT-001 (Cross-Platform)** | ✅ All platforms supported |
| **NFR-BUILD-001 (Build Time)** | ✅ Fast, parallel builds |
| **NFR-MAINT-001 (Testability)** | ✅ Easy to add tests |

## References
- **CMake Documentation**: https://cmake.org/documentation/
- **vcpkg**: https://github.com/microsoft/vcpkg
- **Conan**: https://conan.io/
- **Project Requirements**: [NFR-performance-quality.md](../../02-requirements/non-functional/NFR-performance-quality.md)

---

*This ADR documents the build system configuration for the DAW Project C++ implementation.*