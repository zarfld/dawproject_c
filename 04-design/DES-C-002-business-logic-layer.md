# Component Design Specification: Business Logic Layer

**Document ID**: DES-C-002  
**Version**: 1.0  
**Date**: October 3, 2025  
**Component**: Business Logic Layer  
**Architecture Reference**: ARC-L-002  

---

## 1. Design Overview

### 1.1 Purpose
The Business Logic Layer implements the core DAW Project domain model, business rules, and processing logic. It provides the central engine for project manipulation, validation, and transformation operations.

### 1.2 Scope
**Included**:
- DAW Project domain model (Project, Track, Clip, Device classes)
- Business rule validation and enforcement
- Project transformation and manipulation operations
- Thread-safe concurrent access patterns
- Model state management and change tracking

**Excluded**:
- Data persistence (handled by Data Access Layer)
- User interface concerns (handled by API Layer)
- Platform-specific operations (handled by Platform Abstraction Layer)

### 1.3 Design Context
- **Architecture Component**: ARC-L-002
- **Related Requirements**: REQ-FILE-001, REQ-FILE-003, US-003
- **Related ADRs**: ADR-001 (Architecture style), ADR-005 (Thread safety), ADR-006 (Memory management)
- **Dependencies**: Data Access Layer (ARC-L-003)

---

## 2. Component Architecture

### 2.1 Component Decomposition

| Sub-Component ID | Name | Responsibility |
|------------------|------|----------------|
| DES-C-002-001 | ProjectEngine | Main project management engine |
| DES-C-002-002 | DomainModel | Core DAW Project entities |
| DES-C-002-003 | ValidationEngine | Business rule validation |
| DES-C-002-004 | TransformationEngine | Project manipulation operations |
| DES-C-002-005 | ConcurrencyManager | Thread safety and state management |

### 2.2 Component Interfaces

#### 2.2.1 Public Interface
```cpp
// Main business logic interface exposed to API Layer
class IProjectEngine {
public:
    virtual ~IProjectEngine() = default;
    
    // Project lifecycle operations
    virtual std::shared_ptr<Project> createProject(const std::string& title) = 0;
    virtual std::shared_ptr<Project> loadProject(const std::filesystem::path& path) = 0;
    virtual void saveProject(const Project& project, const std::filesystem::path& path) = 0;
    
    // Project manipulation operations
    virtual void addTrack(Project& project, std::unique_ptr<Track> track) = 0;
    virtual void removeTrack(Project& project, const std::string& trackId) = 0;
    virtual void moveTrack(Project& project, const std::string& trackId, size_t newPosition) = 0;
    
    // Validation operations
    virtual ValidationResult validateProject(const Project& project) = 0;
    virtual std::vector<BusinessRuleViolation> checkBusinessRules(const Project& project) = 0;
    
    // Concurrent access support
    virtual std::shared_ptr<const Project> getReadOnlyView(const std::string& projectId) = 0;
    virtual void beginTransaction(const std::string& projectId) = 0;
    virtual void commitTransaction(const std::string& projectId) = 0;
};
```

#### 2.2.2 Internal Interfaces
```cpp
// Domain model interfaces
class IProjectValidator {
public:
    virtual ~IProjectValidator() = default;
    virtual ValidationResult validate(const Project& project) = 0;
    virtual std::vector<BusinessRuleViolation> checkRules(const Project& project) = 0;
};

class ITransformationEngine {
public:
    virtual ~ITransformationEngine() = default;
    virtual void applyTransformation(Project& project, const ITransformation& transformation) = 0;
    virtual bool canUndo() const = 0;
    virtual bool canRedo() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

class IConcurrencyManager {
public:
    virtual ~IConcurrencyManager() = default;
    virtual std::shared_lock<std::shared_mutex> acquireReadLock(const std::string& projectId) = 0;
    virtual std::unique_lock<std::shared_mutex> acquireWriteLock(const std::string& projectId) = 0;
    virtual void registerProject(const std::string& projectId, std::shared_ptr<Project> project) = 0;
    virtual void unregisterProject(const std::string& projectId) = 0;
};
```

---

## 3. Data Design

### 3.1 Data Structures
```cpp
// Core domain model classes
class Project {
public:
    Project(const std::string& title);
    
    // Accessors
    const std::string& getId() const { return id_; }
    const std::string& getTitle() const { return metadata_.title; }
    const ProjectMetadata& getMetadata() const { return metadata_; }
    const std::vector<std::shared_ptr<Track>>& getTracks() const { return tracks_; }
    
    // Mutators (copy-on-write for thread safety)
    void setTitle(const std::string& title);
    void addTrack(std::shared_ptr<Track> track);
    void removeTrack(const std::string& trackId);
    void moveTrack(const std::string& trackId, size_t newPosition);
    
    // State management
    bool isDirty() const { return dirty_; }
    void markClean() { dirty_ = false; }
    
private:
    std::string id_;
    ProjectMetadata metadata_;
    std::vector<std::shared_ptr<Track>> tracks_;
    std::unordered_map<std::string, std::shared_ptr<Clip>> clips_;
    std::unordered_map<std::string, std::shared_ptr<Device>> devices_;
    mutable std::shared_mutex mutex_;
    std::atomic<bool> dirty_{false};
};

class Track {
public:
    Track(const std::string& name, TrackType type);
    
    // Accessors
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    TrackType getType() const { return type_; }
    const std::vector<std::string>& getClipIds() const { return clipIds_; }
    
    // Mutators
    void setName(const std::string& name) { name_ = name; }
    void addClip(const std::string& clipId);
    void removeClip(const std::string& clipId);
    
private:
    std::string id_;
    std::string name_;
    TrackType type_;
    std::vector<std::string> clipIds_;
    std::vector<std::shared_ptr<Device>> deviceChain_;
    TrackProperties properties_;
};

// Business rule violation structure
struct BusinessRuleViolation {
    RuleType type;
    Severity severity;
    std::string message;
    std::vector<std::string> affectedElementIds;
    std::optional<std::string> suggestedFix;
};

// Transaction state for undo/redo
class ProjectTransaction {
public:
    void addCommand(std::unique_ptr<ICommand> command);
    void execute();
    void undo();
    bool canUndo() const;
    
private:
    std::vector<std::unique_ptr<ICommand>> commands_;
    bool executed_{false};
};
```

### 3.2 Data Flow
1. **Load Flow**: Data Access → Domain Model construction → Validation → Business Logic ready
2. **Manipulation Flow**: API request → Validation → Transformation → State update → Change notification
3. **Save Flow**: Domain Model → Data serialization → Data Access Layer → Persistence

### 3.3 Data Storage
- **In-Memory**: All active projects held in memory as domain objects
- **Copy-on-Write**: Modifications create new versions for thread safety
- **State Tracking**: Dirty flags and change notifications for optimization
- **Transaction Log**: Command history for undo/redo operations

---

## 4. Algorithm Design

### 4.1 Primary Algorithms

**Project Loading Algorithm**:
```cpp
std::shared_ptr<Project> ProjectEngine::loadProject(const std::filesystem::path& path) {
    // 1. Load raw data from Data Access Layer
    auto rawData = dataAccessLayer_->loadProject(path);
    
    // 2. Convert to domain model
    auto project = createProjectFromData(*rawData);
    
    // 3. Validate business rules
    auto violations = validationEngine_->checkRules(*project);
    if (hasBlockingViolations(violations)) {
        throw BusinessRuleException(violations);
    }
    
    // 4. Register for concurrent access
    concurrencyManager_->registerProject(project->getId(), project);
    
    // 5. Mark as clean state
    project->markClean();
    
    return project;
}
```

**Track Manipulation Algorithm**:
```cpp
void ProjectEngine::addTrack(Project& project, std::unique_ptr<Track> track) {
    // 1. Acquire write lock
    auto lock = concurrencyManager_->acquireWriteLock(project.getId());
    
    // 2. Validate business rules for new track
    validateTrackAddition(project, *track);
    
    // 3. Create transformation command
    auto command = std::make_unique<AddTrackCommand>(project.getId(), std::move(track));
    
    // 4. Execute transformation
    transformationEngine_->applyTransformation(project, *command);
    
    // 5. Mark project as dirty
    project.markDirty();
    
    // 6. Notify observers
    notifyProjectChanged(project.getId(), ChangeType::TrackAdded);
}
```

### 4.2 Algorithm Complexity
- **Project Loading**: O(n) where n = number of project elements
- **Track Operations**: O(1) for add/remove, O(n) for reordering where n = track count
- **Validation**: O(m) where m = number of validation rules
- **Copy-on-Write**: O(k) where k = size of modified data structure

### 4.3 Performance Considerations
- **Memory Usage**: Immutable data structures increase memory usage by ~20-30%
- **Copy-on-Write**: Only modified portions are copied, not entire project
- **Validation Caching**: Rule results cached until next modification
- **Lock Granularity**: Per-project locking minimizes contention

---

## 5. Interface Design

### 5.1 API Specification

```cpp
/**
 * @brief Creates a new empty project with given title
 * @param title Project title
 * @return Shared pointer to new project instance
 * @throws InvalidArgumentException If title is empty
 */
std::shared_ptr<Project> IProjectEngine::createProject(const std::string& title);

/**
 * @brief Loads a project from file
 * @param path Path to project file
 * @return Shared pointer to loaded project
 * @throws FileNotFoundException If file doesn't exist
 * @throws BusinessRuleException If project violates business rules
 */
std::shared_ptr<Project> IProjectEngine::loadProject(const std::filesystem::path& path);

/**
 * @brief Adds a track to the project
 * @param project Project to modify
 * @param track Track to add (ownership transferred)
 * @throws BusinessRuleException If track addition violates rules
 * @throws ConcurrencyException If project is locked by another thread
 */
void IProjectEngine::addTrack(Project& project, std::unique_ptr<Track> track);

/**
 * @brief Gets a read-only view of the project for concurrent access
 * @param projectId Project identifier
 * @return Shared pointer to read-only project view
 * @throws ProjectNotFoundException If project ID not found
 */
std::shared_ptr<const Project> IProjectEngine::getReadOnlyView(const std::string& projectId);
```

### 5.2 Error Handling
- **Business Rule Violations**: BusinessRuleException with detailed violation list
- **Concurrency Errors**: ConcurrencyException when locks cannot be acquired
- **State Errors**: InvalidStateException for operations on invalid project state
- **Validation Errors**: ValidationException for data integrity issues

### 5.3 Thread Safety
- **Reader-Writer Locks**: Multiple concurrent readers, exclusive writers per project
- **Copy-on-Write**: Modifications create new immutable versions
- **Atomic Flags**: Dirty state and other flags use atomic operations
- **Lock Ordering**: Consistent lock acquisition order prevents deadlocks

---

## 6. Dependencies

### 6.1 Component Dependencies
| Component | Interface Used | Purpose |
|-----------|----------------|---------|
| ARC-L-003 | IDataAccessLayer | Project persistence operations |

### 6.2 External Dependencies
| Library | Version | Purpose |
|---------|---------|---------|
| std::shared_mutex | C++17 | Reader-writer synchronization |
| std::atomic | C++17 | Atomic operations for flags |
| std::memory | C++17 | Smart pointer management |

---

## 7. Test-Driven Design

### 7.1 Test Strategy
- **Unit Tests**: Each domain class tested independently
- **Integration Tests**: End-to-end business logic scenarios
- **Concurrency Tests**: Multi-threaded access patterns
- **Performance Tests**: Memory usage and operation timing

### 7.2 Key Test Scenarios
```cpp
TEST_CASE("Project Engine - Create and Manipulate Project") {
    auto engine = createProjectEngine();
    auto project = engine->createProject("Test Project");
    
    REQUIRE(project->getTitle() == "Test Project");
    REQUIRE(project->getTracks().empty());
    
    auto track = std::make_unique<Track>("Lead Vocals", TrackType::Audio);
    engine->addTrack(*project, std::move(track));
    
    REQUIRE(project->getTracks().size() == 1);
    REQUIRE(project->isDirty());
}

TEST_CASE("Project Engine - Concurrent Access") {
    auto engine = createProjectEngine();
    auto project = engine->createProject("Concurrent Test");
    
    // Multiple readers should not block
    std::vector<std::thread> readers;
    for (int i = 0; i < 10; ++i) {
        readers.emplace_back([&]() {
            auto readView = engine->getReadOnlyView(project->getId());
            REQUIRE(readView->getTitle() == "Concurrent Test");
        });
    }
    
    for (auto& reader : readers) {
        reader.join();
    }
}

TEST_CASE("Project Engine - Business Rule Validation") {
    auto engine = createProjectEngine();
    auto project = engine->createProject("Rule Test");
    
    // Test rule: Cannot have more than 128 tracks
    for (int i = 0; i < 129; ++i) {
        auto track = std::make_unique<Track>("Track " + std::to_string(i), TrackType::Audio);
        if (i < 128) {
            REQUIRE_NOTHROW(engine->addTrack(*project, std::move(track)));
        } else {
            REQUIRE_THROWS_AS(engine->addTrack(*project, std::move(track)), BusinessRuleException);
        }
    }
}
```

### 7.3 Mock Interfaces
- **MockDataAccessLayer**: For testing without actual file I/O
- **MockValidationEngine**: For testing validation scenarios
- **MockConcurrencyManager**: For testing threading behavior

---

## 8. XP Design Principles

### 8.1 YAGNI Compliance
- Only implement basic CRUD operations initially
- No complex transformation pipeline until needed
- No advanced undo/redo until basic operations are stable

### 8.2 Refactoring Readiness
- Strategy pattern for validation rules allows easy addition
- Command pattern for transformations supports undo/redo evolution
- Interface-based design enables implementation swapping

### 8.3 Simple Design
- Domain objects follow simple property patterns
- Clear separation of concerns (model vs. operations)
- Minimal inheritance (prefer composition and interfaces)

---

## 9. Implementation Notes

### 9.1 Implementation Priority
1. **Core Domain Model**: Project, Track, Clip basic classes
2. **ProjectEngine**: Basic CRUD operations
3. **ValidationEngine**: Essential business rules
4. **ConcurrencyManager**: Thread safety implementation
5. **TransformationEngine**: Undo/redo support

### 9.2 Technology Choices
- **Copy-on-Write**: For thread safety with minimal performance impact
- **Shared Pointers**: For automatic memory management
- **Observer Pattern**: For change notification
- **Command Pattern**: For undo/redo operations

### 9.3 Known Limitations
- Copy-on-write increases memory usage for large projects
- Per-project locking may not scale to hundreds of concurrent projects
- Business rules are hardcoded initially (no rule engine)

---

## 10. Traceability

### 10.1 Requirements Traceability
| Requirement | Design Element | Implementation Note |
|-------------|----------------|-------------------|
| REQ-FILE-001 | ProjectEngine::loadProject() | Implements project loading logic |
| REQ-FILE-003 | ConcurrencyManager | Enables thread-safe operations |
| US-003 | TransformationEngine | Supports project editing operations |
| NFR-THREAD-001 | Copy-on-write + shared_mutex | Thread safety implementation |

### 10.2 Architecture Traceability
| Architecture Element | Design Element | Relationship |
|---------------------|----------------|--------------|
| ARC-L-002 | DES-C-002 | Realizes business logic layer |
| ADR-005 | ConcurrencyManager | Implements thread safety strategy |
| ADR-006 | Smart pointer usage | Implements memory management strategy |

---

*This document follows IEEE 1016-2009 Software Design Descriptions standard.*