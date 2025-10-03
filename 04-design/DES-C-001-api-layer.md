# Component Design Specification: API Layer

**Document ID**: DES-C-001  
**Version**: 1.0  
**Date**: October 3, 2025  
**Component**: API Layer  
**Architecture Reference**: ARC-L-001  

---

## 1. Design Overview

### 1.1 Purpose
The API Layer provides the public interfaces for the DAW Project C++ Library, implementing both modern C++ and C-style APIs as defined in ADR-004. It serves as the primary entry point for all client applications and handles API-level concerns like parameter validation, error translation, and resource management.

### 1.2 Scope
**Included**:
- Modern C++ API with RAII and exception handling
- C-style API with handle management and error codes
- API parameter validation and sanitization
- Error translation between internal exceptions and API error codes
- Resource lifetime management for C API handles
- Thread-safe API operations

**Excluded**:
- Business logic implementation (handled by Business Logic Layer)
- Data persistence operations (handled by Data Access Layer)
- Platform-specific implementations (handled by Platform Abstraction Layer)

### 1.3 Design Context
- **Architecture Component**: ARC-L-001
- **Related Requirements**: REQ-FILE-001, US-001, US-002, NFR-USABILITY-001
- **Related ADRs**: ADR-004 (Dual API strategy), ADR-007 (Error handling)
- **Dependencies**: Business Logic Layer (ARC-L-002)

---

## 2. Component Architecture

### 2.1 Component Decomposition

| Sub-Component ID | Name | Responsibility |
|------------------|------|----------------|
| DES-C-001-001 | CppAPIFacade | Modern C++ public interface |
| DES-C-001-002 | CStyleAPIBridge | C-compatible interface bridge |
| DES-C-001-003 | HandleManager | C API handle lifecycle management |
| DES-C-001-004 | ErrorTranslator | Exception to error code translation |
| DES-C-001-005 | ParameterValidator | Input validation and sanitization |

### 2.2 Component Interfaces

#### 2.2.1 Public Interfaces (C++)
```cpp
// Modern C++ API namespace
namespace dawproject {
    
    class Project {
    public:
        // Factory methods
        static std::unique_ptr<Project> load(const std::filesystem::path& path);
        static std::unique_ptr<Project> create(const std::string& title);
        
        // Project operations
        void save(const std::filesystem::path& path);
        void save(); // Save to original path
        
        // Metadata accessors
        const std::string& getTitle() const;
        void setTitle(const std::string& title);
        const std::string& getArtist() const;
        void setArtist(const std::string& artist);
        double getTempo() const;
        void setTempo(double tempo);
        
        // Track operations
        const std::vector<Track>& getTracks() const;
        void addTrack(std::unique_ptr<Track> track);
        void removeTrack(const std::string& trackId);
        void moveTrack(const std::string& trackId, size_t newPosition);
        std::optional<Track> findTrack(const std::string& name) const;
        
        // Iteration support
        auto begin() const { return tracks_.begin(); }
        auto end() const { return tracks_.end(); }
        
        // Status
        bool isDirty() const;
        
    private:
        Project(std::shared_ptr<internal::IProjectEngine> engine, 
                std::shared_ptr<internal::Project> project);
        
        std::shared_ptr<internal::IProjectEngine> engine_;
        std::shared_ptr<internal::Project> project_;
        std::filesystem::path originalPath_;
    };
    
    class Track {
    public:
        // Accessors
        const std::string& getId() const;
        const std::string& getName() const;
        void setName(const std::string& name);
        TrackType getType() const;
        
        // Properties
        bool isMuted() const;
        void setMuted(bool muted);
        bool isSoloed() const;
        void setSoloed(bool soloed);
        double getVolume() const;
        void setVolume(double volume);
        
    private:
        friend class Project;
        Track(std::shared_ptr<internal::Track> track);
        std::shared_ptr<internal::Track> track_;
    };
}
```

#### 2.2.2 Public Interfaces (C)
```c
// C-style API header
#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle types
typedef struct dawproject_project_t* dawproject_project_handle;
typedef struct dawproject_track_t* dawproject_track_handle;

// Error codes
typedef enum {
    DAWPROJECT_OK = 0,
    DAWPROJECT_ERROR_FILE_NOT_FOUND = 1,
    DAWPROJECT_ERROR_INVALID_FORMAT = 2,
    DAWPROJECT_ERROR_OUT_OF_MEMORY = 3,
    DAWPROJECT_ERROR_INVALID_PARAMETER = 4,
    DAWPROJECT_ERROR_IO_ERROR = 5,
    DAWPROJECT_ERROR_BUSINESS_RULE_VIOLATION = 6,
    DAWPROJECT_ERROR_UNKNOWN = 99
} dawproject_error_t;

// Project management functions
dawproject_error_t dawproject_create(const char* title, dawproject_project_handle* project);
dawproject_error_t dawproject_load(const char* filepath, dawproject_project_handle* project);
dawproject_error_t dawproject_save(dawproject_project_handle project, const char* filepath);
dawproject_error_t dawproject_save_current(dawproject_project_handle project);
void dawproject_project_free(dawproject_project_handle project);

// Project metadata functions
dawproject_error_t dawproject_get_title(dawproject_project_handle project, char* buffer, size_t buffer_size);
dawproject_error_t dawproject_set_title(dawproject_project_handle project, const char* title);
dawproject_error_t dawproject_get_artist(dawproject_project_handle project, char* buffer, size_t buffer_size);
dawproject_error_t dawproject_set_artist(dawproject_project_handle project, const char* artist);
dawproject_error_t dawproject_get_tempo(dawproject_project_handle project, double* tempo);
dawproject_error_t dawproject_set_tempo(dawproject_project_handle project, double tempo);

// Track management functions
dawproject_error_t dawproject_get_track_count(dawproject_project_handle project, int* count);
dawproject_error_t dawproject_get_track(dawproject_project_handle project, int index, dawproject_track_handle* track);
dawproject_error_t dawproject_add_track(dawproject_project_handle project, const char* name, int type);
dawproject_error_t dawproject_remove_track(dawproject_project_handle project, const char* track_id);

// Track property functions
dawproject_error_t dawproject_track_get_name(dawproject_track_handle track, char* buffer, size_t buffer_size);
dawproject_error_t dawproject_track_set_name(dawproject_track_handle track, const char* name);
dawproject_error_t dawproject_track_get_muted(dawproject_track_handle track, int* muted);
dawproject_error_t dawproject_track_set_muted(dawproject_track_handle track, int muted);

// Utility functions
const char* dawproject_error_string(dawproject_error_t error);
dawproject_error_t dawproject_get_version(char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif
```

---

## 3. Data Design

### 3.1 Data Structures
```cpp
// Internal API implementation structures
namespace dawproject::internal {
    
    // Handle management structure
    struct APIHandle {
        enum class Type { Project, Track, Clip, Device };
        
        Type type;
        std::shared_ptr<void> object;
        std::string id;
        std::chrono::time_point<std::chrono::steady_clock> created;
        std::atomic<int> refCount{1};
    };
    
    // Handle registry for C API
    class HandleRegistry {
    public:
        std::string registerHandle(std::shared_ptr<void> object, APIHandle::Type type);
        std::shared_ptr<void> getObject(const std::string& handleId, APIHandle::Type expectedType);
        bool releaseHandle(const std::string& handleId);
        void cleanup(); // Clean up expired handles
        
    private:
        std::unordered_map<std::string, std::unique_ptr<APIHandle>> handles_;
        mutable std::shared_mutex mutex_;
    };
    
    // Error context for detailed error reporting
    struct ErrorContext {
        dawproject_error_t errorCode;
        std::string message;
        std::string sourceFile;
        int sourceLine;
        std::optional<std::string> additionalInfo;
    };
}
```

### 3.2 Data Flow
1. **C++ API Flow**: Client → C++ API → Business Logic → Result → C++ API → Client
2. **C API Flow**: Client → C API → Handle lookup → C++ API → Business Logic → Result → Error translation → C API → Client
3. **Error Flow**: Exception → ErrorTranslator → Error code + context → Client

### 3.3 Data Storage
- **Handle Registry**: Thread-safe map of handle IDs to objects
- **Error Context**: Thread-local storage for detailed error information
- **API State**: Minimal state tracking for handle lifecycle management

---

## 4. Algorithm Design

### 4.1 Primary Algorithms

**C++ API Project Loading**:
```cpp
std::unique_ptr<Project> Project::load(const std::filesystem::path& path) {
    try {
        // 1. Validate input parameters
        ParameterValidator::validateFilePath(path);
        
        // 2. Load project through business logic
        auto engine = BusinessLogicFactory::createProjectEngine();
        auto internalProject = engine->loadProject(path);
        
        // 3. Create API wrapper
        auto apiProject = std::unique_ptr<Project>(new Project(engine, internalProject));
        apiProject->originalPath_ = path;
        
        return apiProject;
        
    } catch (const std::exception& e) {
        // Convert to appropriate API exception
        throw APIException::fromInternalException(e);
    }
}
```

**C API Handle Management**:
```cpp
dawproject_error_t dawproject_load(const char* filepath, dawproject_project_handle* project) {
    return ErrorTranslator::translateException([&]() {
        // 1. Validate parameters
        if (!filepath || !project) {
            throw InvalidParameterException("Null pointer parameter");
        }
        
        // 2. Load using C++ API
        auto cppProject = dawproject::Project::load(filepath);
        
        // 3. Create handle and register
        auto& registry = HandleManager::getInstance();
        auto handleId = registry.registerHandle(std::move(cppProject), APIHandle::Type::Project);
        
        // 4. Convert handle ID to opaque pointer
        *project = reinterpret_cast<dawproject_project_handle>(
            new std::string(handleId)
        );
    });
}
```

### 4.2 Algorithm Complexity
- **C++ API Operations**: O(1) overhead over business logic operations
- **C API Operations**: O(1) handle lookup + C++ API overhead
- **Handle Management**: O(1) registration/lookup using hash table
- **Error Translation**: O(1) exception type checking and conversion

### 4.3 Performance Considerations
- **Handle Lookup**: Constant-time hash table operations
- **Memory Overhead**: Minimal wrapper objects, shared pointers to business logic
- **Error Translation**: Pre-computed error message lookup tables
- **Thread Safety**: Reader-writer locks for handle registry minimize contention

---

## 5. Interface Design

### 5.1 API Specification

```cpp
/**
 * @brief Loads a DAW Project file
 * @param path Path to .dawproject file
 * @return Unique pointer to loaded project
 * @throws FileNotFoundException If file doesn't exist or can't be read
 * @throws ValidationException If file format is invalid
 * @throws BusinessRuleException If project violates business rules
 * 
 * @example
 * auto project = dawproject::Project::load("my_project.dawproject");
 * for (const auto& track : project->getTracks()) {
 *     std::cout << track.getName() << std::endl;
 * }
 */
std::unique_ptr<Project> Project::load(const std::filesystem::path& path);

/**
 * @brief Creates a new empty project
 * @param title Project title (cannot be empty)
 * @return Unique pointer to new project
 * @throws InvalidParameterException If title is empty
 * 
 * @example
 * auto project = dawproject::Project::create("My New Song");
 * project->setTempo(120.0);
 * project->save("new_project.dawproject");
 */
std::unique_ptr<Project> Project::create(const std::string& title);
```

For C API:
```c
/**
 * @brief Loads a DAW Project file
 * @param filepath Path to .dawproject file (must be null-terminated)
 * @param project Output parameter for project handle
 * @return DAWPROJECT_OK on success, error code on failure
 * 
 * @example
 * dawproject_project_handle project = NULL;
 * dawproject_error_t error = dawproject_load("my_project.dawproject", &project);
 * if (error == DAWPROJECT_OK) {
 *     // Use project...
 *     dawproject_project_free(project);
 * }
 */
dawproject_error_t dawproject_load(const char* filepath, dawproject_project_handle* project);
```

### 5.2 Error Handling
- **C++ API**: Exception-based with custom exception hierarchy
- **C API**: Error codes with optional detailed error message access
- **Error Context**: Thread-local error context for detailed diagnostics
- **Error Messages**: Localized error messages (English initially)

### 5.3 Thread Safety
- **C++ API**: Thread-safe through underlying business logic thread safety
- **C API**: Thread-safe handle management with reader-writer locks
- **Handle Registry**: Concurrent access to handle lookup operations
- **Error Context**: Thread-local storage prevents race conditions

---

## 6. Dependencies

### 6.1 Component Dependencies
| Component | Interface Used | Purpose |
|-----------|----------------|---------|
| ARC-L-002 | IProjectEngine | Core business logic operations |

### 6.2 External Dependencies
| Library | Version | Purpose |
|---------|---------|---------|
| std::filesystem | C++17 | File path operations |
| std::memory | C++17 | Smart pointer management |
| std::shared_mutex | C++17 | Thread synchronization |

---

## 7. Test-Driven Design

### 7.1 Test Strategy
- **Unit Tests**: Each API method tested independently
- **Integration Tests**: API + Business Logic integration scenarios
- **Error Handling Tests**: All error conditions and exception paths
- **Thread Safety Tests**: Concurrent API access patterns
- **API Equivalence Tests**: C++ and C APIs produce identical results

### 7.2 Key Test Scenarios
```cpp
TEST_CASE("C++ API - Project Lifecycle") {
    auto project = dawproject::Project::create("Test Project");
    REQUIRE(project->getTitle() == "Test Project");
    REQUIRE(project->getTracks().empty());
    REQUIRE(project->isDirty() == false);
    
    project->setTitle("Modified Title");
    REQUIRE(project->isDirty() == true);
    
    project->save("test_output.dawproject");
    REQUIRE(project->isDirty() == false);
}

TEST_CASE("C API - Project Lifecycle") {
    dawproject_project_handle project = nullptr;
    
    // Create project
    REQUIRE(dawproject_create("Test Project", &project) == DAWPROJECT_OK);
    
    // Check title
    char title[256];
    REQUIRE(dawproject_get_title(project, title, sizeof(title)) == DAWPROJECT_OK);
    REQUIRE(std::string(title) == "Test Project");
    
    // Cleanup
    dawproject_project_free(project);
}

TEST_CASE("API Equivalence - Load and Access") {
    const char* testFile = "test_project.dawproject";
    
    // Test C++ API
    auto cppProject = dawproject::Project::load(testFile);
    auto cppTrackCount = cppProject->getTracks().size();
    
    // Test C API
    dawproject_project_handle cProject = nullptr;
    REQUIRE(dawproject_load(testFile, &cProject) == DAWPROJECT_OK);
    
    int cTrackCount = 0;
    REQUIRE(dawproject_get_track_count(cProject, &cTrackCount) == DAWPROJECT_OK);
    
    // Should be equivalent
    REQUIRE(cppTrackCount == static_cast<size_t>(cTrackCount));
    
    dawproject_project_free(cProject);
}
```

### 7.3 Mock Interfaces
- **MockProjectEngine**: For testing API layer independently
- **MockHandleRegistry**: For testing handle management
- **MockErrorTranslator**: For testing error handling scenarios

---

## 8. XP Design Principles

### 8.1 YAGNI Compliance
- Only implement essential API methods initially
- No advanced features until basic operations are stable
- No optimization until performance issues are identified

### 8.2 Refactoring Readiness
- Facade pattern allows easy underlying implementation changes
- Handle management abstraction enables different storage strategies
- Interface segregation supports adding specialized APIs

### 8.3 Simple Design
- Minimal API surface covering core use cases
- Clear method names and parameter patterns
- Consistent error handling across all operations

---

## 9. Implementation Notes

### 9.1 Implementation Priority
1. **Core C++ API**: Project create/load/save operations
2. **Handle Management**: C API handle registry
3. **Error Translation**: Exception to error code mapping
4. **C API Bridge**: Basic C operations
5. **Advanced Operations**: Track manipulation, validation

### 9.2 Technology Choices
- **RAII Wrappers**: For automatic resource management in C++ API
- **Opaque Handles**: For C API type safety
- **Smart Pointers**: For memory management
- **Template Metaprogramming**: For compile-time API validation

### 9.3 Known Limitations
- C API requires manual memory management by clients
- Handle registry has memory overhead for C API usage
- No automatic handle cleanup (requires explicit free calls)

---

## 10. Traceability

### 10.1 Requirements Traceability
| Requirement | Design Element | Implementation Note |
|-------------|----------------|-------------------|
| REQ-FILE-001 | Project::load()/dawproject_load() | Implements project loading |
| US-001 | C++ API facade pattern | Simple, intuitive API design |
| US-002 | Project::save()/dawproject_save() | Implements project saving |
| NFR-USABILITY-001 | API design patterns | Developer-friendly interface |

### 10.2 Architecture Traceability
| Architecture Element | Design Element | Relationship |
|---------------------|----------------|--------------|
| ARC-L-001 | DES-C-001 | Realizes API layer |
| ADR-004 | Dual API implementation | Implements dual API strategy |
| ADR-007 | ErrorTranslator | Implements error handling approach |

---

*This document follows IEEE 1016-2009 Software Design Descriptions standard.*