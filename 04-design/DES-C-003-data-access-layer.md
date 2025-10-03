# Component Design Specification: Data Access Layer

**Document ID**: DES-C-003  
**Version**: 1.0  
**Date**: October 3, 2025  
**Component**: Data Access Layer  
**Architecture Reference**: ARC-L-003  

---

## 1. Design Overview

### 1.1 Purpose
The Data Access Layer provides unified XML/ZIP I/O operations for DAW Project files, abstracting the complexities of pugixml and minizip libraries while supporting both DOM and streaming access patterns.

### 1.2 Scope
**Included**:
- XML parsing and generation using pugixml
- ZIP archive reading and writing using minizip
- File reference resolution and validation
- DOM and streaming access pattern implementations
- Error handling and validation

**Excluded**:
- Business logic validation (handled by Business Logic Layer)
- API surface design (handled by API Layer)
- Platform-specific file operations (handled by Platform Abstraction Layer)

### 1.3 Design Context
- **Architecture Component**: ARC-L-003
- **Related Requirements**: REQ-FILE-001, REQ-FILE-002, REQ-FILE-003
- **Related ADRs**: ADR-002 (XML parser), ADR-003 (ZIP library)
- **Dependencies**: Platform Abstraction Layer (ARC-L-004)

---

## 2. Component Architecture

### 2.1 Component Decomposition

| Sub-Component ID | Name | Responsibility |
|------------------|------|----------------|
| DES-C-003-001 | XMLProcessor | XML parsing and generation |
| DES-C-003-002 | ZIPProcessor | ZIP archive operations |
| DES-C-003-003 | FileResolver | File reference resolution |
| DES-C-003-004 | AccessStrategyFactory | DOM/streaming pattern factory |
| DES-C-003-005 | ValidationEngine | File format validation |

### 2.2 Component Interfaces

#### 2.2.1 Public Interface
```cpp
// Main interface exposed to Business Logic Layer
class IDataAccessLayer {
public:
    virtual ~IDataAccessLayer() = default;
    
    // Project file operations
    virtual std::unique_ptr<ProjectData> loadProject(
        const std::filesystem::path& path, 
        AccessMode mode = AccessMode::DOM) = 0;
    
    virtual void saveProject(
        const ProjectData& project, 
        const std::filesystem::path& path) = 0;
    
    // Streaming operations
    virtual std::unique_ptr<IProjectStream> openStream(
        const std::filesystem::path& path) = 0;
    
    // Validation operations
    virtual ValidationResult validateProject(
        const std::filesystem::path& path) = 0;
};
```

#### 2.2.2 Internal Interfaces
```cpp
// Strategy pattern for access modes
class IAccessStrategy {
public:
    virtual ~IAccessStrategy() = default;
    virtual std::unique_ptr<ProjectData> load(const std::filesystem::path& path) = 0;
    virtual void save(const ProjectData& project, const std::filesystem::path& path) = 0;
    virtual bool supportsStreaming() const = 0;
};

// XML processing interface
class IXMLProcessor {
public:
    virtual ~IXMLProcessor() = default;
    virtual pugi::xml_document parseXML(std::istream& input) = 0;
    virtual void generateXML(const ProjectData& data, std::ostream& output) = 0;
    virtual ValidationResult validateXML(const pugi::xml_document& doc) = 0;
};

// ZIP processing interface
class IZIPProcessor {
public:
    virtual ~IZIPProcessor() = default;
    virtual std::unique_ptr<ZIPReader> openForReading(const std::filesystem::path& path) = 0;
    virtual std::unique_ptr<ZIPWriter> openForWriting(const std::filesystem::path& path) = 0;
    virtual std::vector<std::string> listEntries(const std::filesystem::path& path) = 0;
};
```

---

## 3. Data Design

### 3.1 Data Structures
```cpp
// Core data transfer objects
struct ProjectData {
    ProjectMetadata metadata;
    std::vector<TrackData> tracks;
    std::vector<ClipData> clips;
    std::vector<DeviceData> devices;
    std::unordered_map<std::string, std::filesystem::path> fileReferences;
};

struct ProjectMetadata {
    std::string title;
    std::string artist;
    double tempo;
    std::string version;
    TimeSignature timeSignature;
    std::chrono::system_clock::time_point createdTime;
    std::chrono::system_clock::time_point modifiedTime;
};

struct TrackData {
    std::string id;
    std::string name;
    TrackType type;
    std::vector<std::string> clipIds;
    std::vector<DeviceChainData> deviceChain;
    bool muted;
    bool soloed;
    double volume;
    double pan;
};

// Validation result structure
struct ValidationResult {
    bool isValid;
    std::vector<ValidationError> errors;
    std::vector<ValidationWarning> warnings;
};

struct ValidationError {
    ErrorType type;
    std::string message;
    std::optional<pugi::xml_node> sourceNode;
    std::optional<size_t> lineNumber;
};
```

### 3.2 Data Flow
1. **Loading Flow**: ZIP → XML extraction → XML parsing → ProjectData construction
2. **Saving Flow**: ProjectData → XML generation → ZIP creation → File writing
3. **Streaming Flow**: ZIP → Selective extraction → On-demand XML parsing → Progressive data

### 3.3 Data Storage
- **Memory**: ProjectData objects held in memory for DOM access
- **Streaming**: Temporary extraction to system temp directory for streaming access
- **Caching**: Parsed XML nodes cached for repeated access in streaming mode

---

## 4. Algorithm Design

### 4.1 Primary Algorithms

**DOM Loading Algorithm**:
```cpp
std::unique_ptr<ProjectData> loadDOM(const std::filesystem::path& path) {
    // 1. Open ZIP archive
    auto zipReader = zipProcessor_->openForReading(path);
    
    // 2. Extract project.xml to memory
    auto xmlStream = zipReader->extractToStream("project.xml");
    
    // 3. Parse XML document
    auto xmlDoc = xmlProcessor_->parseXML(*xmlStream);
    
    // 4. Validate XML structure
    auto validation = xmlProcessor_->validateXML(xmlDoc);
    if (!validation.isValid) {
        throw ValidationException(validation.errors);
    }
    
    // 5. Convert XML to ProjectData
    return convertXMLToProjectData(xmlDoc);
}
```

**Streaming Loading Algorithm**:
```cpp
std::unique_ptr<IProjectStream> openStream(const std::filesystem::path& path) {
    // 1. Validate ZIP structure without full extraction
    auto entries = zipProcessor_->listEntries(path);
    validateZIPStructure(entries);
    
    // 2. Create streaming reader with lazy loading
    return std::make_unique<ProjectStreamReader>(path, zipProcessor_, xmlProcessor_);
}
```

### 4.2 Algorithm Complexity
- **DOM Loading**: O(n) where n = total XML size, full document in memory
- **Streaming Loading**: O(1) initial overhead, O(k) per accessed element where k = element size
- **Validation**: O(n) where n = XML document size
- **ZIP Operations**: O(m) where m = number of ZIP entries

### 4.3 Performance Considerations
- **Memory Usage**: DOM mode ~5-10MB for typical projects, streaming mode <50MB bounded
- **Load Time**: DOM parsing dominates, ~100-500ms for 32-track projects
- **I/O Optimization**: Minimize ZIP entry extraction, cache frequently accessed data

---

## 5. Interface Design

### 5.1 API Specification

```cpp
/**
 * @brief Loads a DAW Project file into memory
 * @param path Path to .dawproject file
 * @param mode Access mode (DOM or streaming)
 * @return Unique pointer to loaded project data
 * @throws FileNotFoundException If file doesn't exist
 * @throws ValidationException If file is malformed
 * @throws ParseException If XML parsing fails
 */
std::unique_ptr<ProjectData> IDataAccessLayer::loadProject(
    const std::filesystem::path& path, 
    AccessMode mode = AccessMode::DOM);

/**
 * @brief Saves project data to a DAW Project file
 * @param project Project data to save
 * @param path Output file path
 * @throws FileWriteException If file cannot be written
 * @throws SerializationException If data cannot be serialized
 */
void IDataAccessLayer::saveProject(
    const ProjectData& project, 
    const std::filesystem::path& path);

/**
 * @brief Opens a streaming interface to a project file
 * @param path Path to .dawproject file
 * @return Streaming interface for progressive loading
 * @throws FileNotFoundException If file doesn't exist
 * @throws ValidationException If ZIP structure is invalid
 */
std::unique_ptr<IProjectStream> IDataAccessLayer::openStream(
    const std::filesystem::path& path);

/**
 * @brief Validates a project file without full loading
 * @param path Path to .dawproject file
 * @return Validation result with errors and warnings
 */
ValidationResult IDataAccessLayer::validateProject(
    const std::filesystem::path& path);
```

### 5.2 Error Handling
- **File Errors**: FileNotFoundException, FileWriteException
- **Format Errors**: ValidationException, ParseException
- **Data Errors**: SerializationException, CorruptedDataException
- **Resource Errors**: OutOfMemoryException, DiskFullException

Exception hierarchy follows ADR-007 error handling strategy with detailed error messages and context.

### 5.3 Thread Safety
- **Read Operations**: Thread-safe using shared_mutex for concurrent reads
- **Write Operations**: Exclusive access using unique lock
- **Streaming**: Each stream maintains independent state, thread-safe per stream
- **Caching**: Thread-safe caching using concurrent data structures

---

## 6. Dependencies

### 6.1 Component Dependencies
| Component | Interface Used | Purpose |
|-----------|----------------|---------|
| ARC-L-004 | IPlatformAbstraction | File system operations |

### 6.2 External Dependencies
| Library | Version | Purpose |
|---------|---------|---------|
| pugixml | 1.13+ | XML parsing and generation |
| minizip | 1.1+ | ZIP archive operations |
| std::filesystem | C++17 | File path operations |

---

## 7. Test-Driven Design

### 7.1 Test Strategy
- **Unit Tests**: Each sub-component tested independently with mocked dependencies
- **Integration Tests**: End-to-end tests with real DAW Project files
- **Performance Tests**: Memory usage and timing tests for large projects
- **Error Tests**: Comprehensive error condition testing with malformed files

### 7.2 Key Test Scenarios
```cpp
TEST_CASE("Data Access Layer - Load Valid Project") {
    auto dataAccess = createDataAccessLayer();
    auto project = dataAccess->loadProject("test_project.dawproject");
    
    REQUIRE(project != nullptr);
    REQUIRE(project->metadata.title == "Test Project");
    REQUIRE(project->tracks.size() == 4);
}

TEST_CASE("Data Access Layer - Handle Malformed ZIP") {
    auto dataAccess = createDataAccessLayer();
    
    REQUIRE_THROWS_AS(
        dataAccess->loadProject("malformed.dawproject"),
        ValidationException
    );
}

TEST_CASE("Data Access Layer - Streaming Access") {
    auto dataAccess = createDataAccessLayer();
    auto stream = dataAccess->openStream("large_project.dawproject");
    
    // Should not consume >50MB memory
    REQUIRE(getCurrentMemoryUsage() < 50 * 1024 * 1024);
    
    auto tracks = stream->getTracks();
    REQUIRE(tracks.size() > 0);
}
```

### 7.3 Mock Interfaces
- **MockZIPProcessor**: For testing without actual ZIP operations
- **MockXMLProcessor**: For testing XML parsing scenarios
- **MockFileSystem**: For testing file system error conditions

---

## 8. XP Design Principles

### 8.1 YAGNI Compliance
- Only implement DOM and streaming access patterns (no others until needed)
- No caching beyond what's required for streaming performance
- No compression level options until performance issues identified

### 8.2 Refactoring Readiness
- Strategy pattern enables easy addition of new access modes
- Interface-based design allows swapping implementations
- Dependency injection supports testing and evolution

### 8.3 Simple Design
- Clear separation of XML, ZIP, and validation concerns
- Minimal inheritance hierarchies (prefer composition)
- Single responsibility per class

---

## 9. Implementation Notes

### 9.1 Implementation Priority
1. **XMLProcessor**: Core XML parsing with pugixml
2. **ZIPProcessor**: Basic ZIP operations with minizip
3. **DOMAccessStrategy**: Full document loading
4. **ValidationEngine**: XML structure validation
5. **StreamingAccessStrategy**: Progressive loading
6. **FileResolver**: Reference resolution

### 9.2 Technology Choices
- **pugixml**: Header-only XML library for easy integration
- **minizip**: Lightweight ZIP library from zlib
- **std::filesystem**: C++17 file operations
- **RAII**: All resource management through smart pointers

### 9.3 Known Limitations
- No XSD schema validation in initial implementation
- Streaming mode requires temporary disk space for ZIP extraction
- Memory usage scales with project complexity in DOM mode

---

## 10. Traceability

### 10.1 Requirements Traceability
| Requirement | Design Element | Implementation Note |
|-------------|----------------|-------------------|
| REQ-FILE-001 | IDataAccessLayer::loadProject() | Implements project loading |
| REQ-FILE-002 | ValidationEngine | Handles malformed files |
| REQ-FILE-003 | StreamingAccessStrategy | Enables bounded memory access |
| NFR-PERF-001 | DOMAccessStrategy | Optimized for <30s load time |
| NFR-PERF-002 | StreamingAccessStrategy | <500MB memory usage |

### 10.2 Architecture Traceability
| Architecture Element | Design Element | Relationship |
|---------------------|----------------|--------------|
| ARC-L-003 | DES-C-003 | Realizes data access layer |
| ADR-002 | XMLProcessor | Implements pugixml decision |
| ADR-003 | ZIPProcessor | Implements minizip decision |

---

*This document follows IEEE 1016-2009 Software Design Descriptions standard.*