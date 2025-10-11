# ADR-009: External Schema Management Strategy

**Status**: PROPOSED  
**Date**: 2025-01-09  
**Deciders**: Development Team, Architecture Review Board  
**Technical Story**: [External Authority Compliance] Implement automated external schema management for DAWProject XSD validation

## Context

Gap analysis revealed that our custom validation approach violates external authority compliance principles. We must validate against authoritative XSD schemas maintained by the official DAWProject repository:

- `Project.xsd` - DAWProject main schema
- `MetaData.xsd` - Metadata structure schema

**Requirements**:
- Automatic schema discovery and download
- Local caching with version control
- Fallback mechanisms for offline scenarios
- Integration with libxml2 validation pipeline

**Constraints**:
- Network connectivity may be limited in some deployment scenarios
- Schema updates must not break existing functionality
- Performance impact must be minimal

## Decision

We will implement **ExternalSchemaManager** with the following strategy:

### Core Components

1. **SchemaDiscoveryService** - Automatic schema detection
2. **SchemaCacheManager** - Local storage and versioning
3. **SchemaUpdateService** - Automated updates with validation
4. **FallbackSchemaProvider** - Embedded backup schemas

### Implementation Strategy

```cpp
class ExternalSchemaManager {
public:
    // Primary interface
    SchemaValidationResult validateWithExternalSchema(
        const std::filesystem::path& xmlFile,
        SchemaType schemaType
    );
    
    // Schema lifecycle management
    void updateSchemas();
    bool checkSchemaUpdates();
    void cacheSchemas();
    
private:
    SchemaDiscoveryService discoveryService_;
    SchemaCacheManager cacheManager_;
    SchemaUpdateService updateService_;
    FallbackSchemaProvider fallbackProvider_;
};
```

### Schema Discovery Protocol

1. **Primary Source**: `https://raw.githubusercontent.com/bitwig/dawproject/main/schemas/`
2. **Discovery Method**: HTTP HEAD requests to check availability
3. **Version Detection**: ETag/Last-Modified headers for change detection
4. **Fallback Chain**: Cache → Embedded → Error

### Caching Strategy

```
~/.dawproject_cpp/schemas/
├── Project.xsd                 # Latest cached Project schema
├── MetaData.xsd               # Latest cached Metadata schema
├── versions/
│   ├── Project_v1.0.xsd      # Versioned schemas
│   └── MetaData_v1.0.xsd
└── cache.json                 # Cache metadata
```

## Alternatives Considered

### ❌ **Embedded Schemas Only**
- **Pros**: No network dependency, faster startup
- **Cons**: Violates external authority principle, requires manual updates
- **Verdict**: Rejected - fails compliance requirements

### ❌ **Always Download**
- **Pros**: Always current schemas
- **Cons**: Network requirement, performance impact, failure points
- **Verdict**: Rejected - too fragile for production

### ❌ **Manual Schema Management**
- **Pros**: Full developer control
- **Cons**: Maintenance burden, update lag, human error risk
- **Verdict**: Rejected - doesn't scale

## Consequences

### Positive

✅ **External Authority Compliance**: Validates against official schemas  
✅ **Automatic Updates**: Schema changes incorporated without code changes  
✅ **Offline Capability**: Cache provides fallback for disconnected scenarios  
✅ **Performance**: Local cache minimizes network overhead  
✅ **Version Control**: Track schema evolution and compatibility  
✅ **Reliability**: Fallback chain ensures validation always available

### Negative

⚠️ **Network Dependency**: Initial setup requires internet connectivity  
**Mitigation**: Embedded fallback schemas for offline-first scenarios  

⚠️ **Cache Management**: Local storage and cleanup complexity  
**Mitigation**: Configurable cache limits with automatic cleanup  

⚠️ **Update Risks**: External schema changes might break compatibility  
**Mitigation**: Version validation and compatibility checks before updates  

⚠️ **Security Considerations**: External downloads need verification  
**Mitigation**: HTTPS with certificate pinning, checksum verification

### Risk Assessment

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| **Schema Unavailable** | High | Low | Fallback chain with embedded schemas |
| **Network Failures** | Medium | Medium | Robust retry logic and caching |
| **Schema Breaking Changes** | High | Low | Version validation before adoption |
| **Cache Corruption** | Medium | Low | Checksum validation and auto-repair |
| **Security Vulnerabilities** | High | Low | HTTPS, certificate pinning, checksums |

## Implementation Plan

### Phase 1: Core Infrastructure
- [ ] ExternalSchemaManager base class
- [ ] SchemaCacheManager with local storage
- [ ] FallbackSchemaProvider with embedded schemas
- [ ] Basic validation integration with libxml2

### Phase 2: Network Operations  
- [ ] SchemaDiscoveryService with HTTP client
- [ ] SchemaUpdateService with version detection
- [ ] Retry logic and error handling
- [ ] Cache cleanup and management

### Phase 3: Production Features
- [ ] Configuration management
- [ ] Logging and monitoring
- [ ] Performance optimization
- [ ] Security hardening

### Phase 4: Integration
- [ ] DAWProjectReader integration
- [ ] DAWProjectWriter validation
- [ ] Error reporting enhancement
- [ ] Documentation and examples

## Configuration

```cpp
struct ExternalSchemaConfig {
    std::string baseUrl = "https://raw.githubusercontent.com/bitwig/dawproject/main/schemas/";
    std::chrono::hours cacheValidDuration{24};
    std::filesystem::path cacheDirectory = "~/.dawproject_cpp/schemas/";
    bool enableAutoUpdates = true;
    bool requireExternalValidation = true;
    size_t maxRetries = 3;
    std::chrono::seconds networkTimeout{30};
};
```

## Monitoring

- **Cache Hit Rate**: Measure local cache effectiveness
- **Update Frequency**: Track schema update patterns  
- **Validation Performance**: Monitor libxml2 validation overhead
- **Network Errors**: Alert on persistent connectivity issues
- **Fallback Usage**: Track embedded schema usage patterns

## References

- **DAWProject Repository**: <https://github.com/bitwig/dawproject>
- **XML Schema Validation**: <https://www.w3.org/TR/xmlschema11-1/>
- **HTTP Caching**: RFC 7234 - HTTP/1.1 Caching
- **libxml2 Schema Validation**: <http://xmlsoft.org/html/libxml-xmlschemas.html>

---

*This ADR establishes external schema management strategy ensuring DAWProject specification compliance while maintaining performance and reliability.*