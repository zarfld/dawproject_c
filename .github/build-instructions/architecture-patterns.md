# Architecture Patterns and Guidelines

**Purpose**: Define architectural abstractions and patterns to prevent hardcoded implementations

## 🏗️ **XML Processing Architecture**

### **XMLProcessor Pattern** ✅

**Rule**: NEVER hardcode pugixml or other XML library calls in business logic

**Correct Pattern**:
```cpp
// 1. Define abstraction interface
class IDawProjectXMLParser {
public:
    virtual Result<ProjectInfo> parseProjectInfo(const XMLDocument& doc) = 0;
    virtual Result<std::vector<TrackInfo>> parseTracks(const XMLDocument& doc) = 0;
    virtual ~IDawProjectXMLParser() = default;
};

// 2. Implement concrete parser
class DawProjectXMLParserImpl : public IDawProjectXMLParser {
public:
    Result<ProjectInfo> parseProjectInfo(const XMLDocument& doc) override {
        // Parse <Project version="1.0"><Application><Transport>
        // Using XMLDocument abstraction, not raw pugixml
    }
};

// 3. Use in business logic
class DataAccessEngineImpl {
    Result<ProjectInfo> loadProjectInfo(const path& filePath) {
        auto doc = xmlProcessor_->loadDocument(filePath);
        return dawProjectParser_->parseProjectInfo(doc.value);
    }
};
```

**❌ Wrong Pattern (What I Did)**:
```cpp
// DON'T DO THIS - hardcoded XML parsing in business logic
Result<ProjectInfo> loadProjectInfo(const path& filePath) {
    pugi::xml_document doc;  // ❌ Hardcoded XML library
    doc.load_file(path);     // ❌ Direct parsing in business method
    auto node = doc.child("Project"); // ❌ Hardcoded XML structure
}
```

### **XMLGenerator Pattern** ✅

**Correct Pattern**:
```cpp
class IDawProjectXMLGenerator {
public:
    virtual XMLDocument generateProject(const ProjectInfo& info, 
                                      const std::vector<TrackInfo>& tracks) = 0;
    virtual XMLDocument generateMetadata(const ProjectInfo& info) = 0;
};

class DawProjectXMLGeneratorImpl : public IDawProjectXMLGenerator {
    XMLDocument generateProject(const ProjectInfo& info, 
                               const std::vector<TrackInfo>& tracks) override {
        // Generate standard-compliant <Project version="1.0"> structure
    }
};
```

## 📦 **Data Access Architecture**

### **DataAccessEngine Pattern** ✅

**Rule**: Separate data access from business logic through clean interfaces

**Correct Pattern**:
```cpp
// 1. Interface defines operations
class IDataAccessEngine {
public:
    virtual Result<ProjectInfo> loadProjectInfo(const path& filePath) = 0;
    virtual Result<std::vector<TrackInfo>> loadTracks(const path& filePath) = 0;
    virtual Result<void> saveProject(const ProjectData& data, const path& filePath) = 0;
};

// 2. Implementation uses abstracted components
class DataAccessEngineImpl : public IDataAccessEngine {
private:
    std::unique_ptr<IXMLProcessor> xmlProcessor_;
    std::unique_ptr<IZipProcessor> zipProcessor_;
    std::unique_ptr<IDawProjectXMLParser> xmlParser_;

public:
    Result<ProjectInfo> loadProjectInfo(const path& filePath) override {
        // 1. Extract XML from ZIP (using zipProcessor_)
        // 2. Parse XML (using xmlParser_)
        // 3. Return validated results
    }
};
```

### **Dependency Injection** ✅

**Rule**: Inject dependencies, don't create them internally

**Correct Pattern**:
```cpp
class DataAccessEngineImpl {
public:
    DataAccessEngineImpl(std::unique_ptr<IXMLProcessor> xmlProc,
                        std::unique_ptr<IZipProcessor> zipProc,
                        std::unique_ptr<IDawProjectXMLParser> parser)
        : xmlProcessor_(std::move(xmlProc))
        , zipProcessor_(std::move(zipProc))
        , xmlParser_(std::move(parser)) {}
};

// Factory creates and injects
std::unique_ptr<IDataAccessEngine> createDataAccessEngine() {
    return std::make_unique<DataAccessEngineImpl>(
        createXMLProcessor(),
        createZipProcessor(), 
        createDawProjectXMLParser()
    );
}
```

## 🗜️ **ZIP Container Architecture**

### **ZipProcessor Pattern** ✅

**Rule**: Abstract ZIP operations for testability and flexibility

**Correct Pattern**:
```cpp
class IZipProcessor {
public:
    virtual Result<std::string> extractFile(const path& zipPath, 
                                          const std::string& fileName) = 0;
    virtual Result<void> addFile(const path& zipPath, 
                                const std::string& fileName, 
                                const std::string& content) = 0;
};

class ZipProcessorImpl : public IZipProcessor {
    Result<std::string> extractFile(const path& zipPath, 
                                   const std::string& fileName) override {
        // Use actual ZIP library (libzip, minizip, etc.)
    }
};

class MockZipProcessor : public IZipProcessor {
    // For testing - simulate ZIP operations
};
```

## 🧪 **Testing Architecture**

### **Test Data Generation** ✅

**Rule**: Generate test data using the same abstractions as production

**Correct Pattern**:
```cpp
class TestDataGenerator {
private:
    std::unique_ptr<IDawProjectXMLGenerator> generator_;
    
public:
    std::string createMinimalValidProject() {
        ProjectInfo info;
        info.title = "Test Project";
        info.tempo = 120.0;
        info.timeSignature = "4/4";
        
        // Use same generator as production
        auto doc = generator_->generateProject(info, {});
        return xmlProcessor_->documentToString(doc);
    }
};
```

**❌ Wrong Pattern**:
```cpp
std::string createMinimalValidProject() {
    // DON'T hardcode XML strings
    return R"(<project><metadata><title>Test</title></metadata></project>)";
}
```

## 🔄 **Error Handling Architecture**

### **Result Pattern** ✅

**Rule**: Use Result<T> for all operations that can fail

**Correct Pattern**:
```cpp
template<typename T>
class Result {
public:
    static Result<T> makeSuccess(T value) { /* ... */ }
    static Result<T> makeError(const std::string& message) { /* ... */ }
    
    bool isSuccess() const { /* ... */ }
    const T& getValue() const { /* ... */ }
    const std::string& getError() const { /* ... */ }
};

// Usage
Result<ProjectInfo> loadProject(const path& filePath) {
    auto zipResult = zipProcessor_->extractFile(filePath, "project.xml");
    if (!zipResult.isSuccess()) {
        return Result<ProjectInfo>::makeError("Failed to extract: " + zipResult.getError());
    }
    
    auto parseResult = xmlParser_->parseProjectInfo(zipResult.getValue());
    if (!parseResult.isSuccess()) {
        return Result<ProjectInfo>::makeError("Parse failed: " + parseResult.getError());
    }
    
    return Result<ProjectInfo>::makeSuccess(parseResult.getValue());
}
```

## 📁 **File Organization Architecture**

### **Component Structure** ✅

```
src/
├── data/
│   ├── interfaces/
│   │   ├── IDataAccessEngine.h
│   │   ├── IXMLProcessor.h
│   │   └── IZipProcessor.h
│   ├── xml/
│   │   ├── IDawProjectXMLParser.h
│   │   ├── DawProjectXMLParserImpl.cpp
│   │   ├── IDawProjectXMLGenerator.h
│   │   └── DawProjectXMLGeneratorImpl.cpp
│   ├── zip/
│   │   ├── ZipProcessorImpl.cpp
│   │   └── MockZipProcessor.cpp (for testing)
│   └── DataAccessEngineImpl.cpp
└── dawproject/
    ├── DawProject.h (public API)
    └── DawProjectImpl.cpp
```

## 🎯 **Validation Architecture**

### **Schema Validation** ✅

**Rule**: Validate against official DAWProject schemas

**Correct Pattern**:
```cpp
class ISchemaValidator {
public:
    virtual Result<void> validateProject(const XMLDocument& doc) = 0;
    virtual Result<void> validateMetadata(const XMLDocument& doc) = 0;
};

class DawProjectSchemaValidator : public ISchemaValidator {
    Result<void> validateProject(const XMLDocument& doc) override {
        // Validate against Project.xsd from bitwig/dawproject
    }
};
```

## 🚨 **Anti-Patterns to Avoid**

### **❌ Hardcoded XML Operations**
```cpp
// DON'T DO THIS
pugi::xml_document doc;
doc.load_file("project.xml");
auto node = doc.child("Project");
```

### **❌ Mixed Concerns**
```cpp
// DON'T DO THIS - mixing ZIP, XML, and business logic
class DawProject {
    bool load(const path& file) {
        ZipFile zip(file);           // ZIP handling
        auto xml = zip.extract(...); // in same method
        pugi::xml_document doc;      // with XML parsing
        doc.load_string(xml);        // and business logic
        title_ = doc.child("title").text(); // all mixed together
    }
};
```

### **❌ Tight Coupling**
```cpp
// DON'T DO THIS - tight coupling to specific implementations
class DataEngine {
    PugiXMLProcessor processor_;  // Hardcoded to pugixml
    LibzipProcessor zipper_;      // Hardcoded to libzip
};
```

---

**⚠️ RULE**: Always code to interfaces, not implementations. Use dependency injection. Separate concerns cleanly.