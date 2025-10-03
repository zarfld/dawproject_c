# Context View

## Purpose
Describes the system's external actors, systems, and boundaries (C4 Level 1).

## Primary Stakeholders
| Stakeholder | Concern |
|-------------|---------|
| Developer | Integration points |
| DAW Vendor | Compatibility |
| End User | Data integrity |

## Model
```mermaid
graph TB
    subgraph "External Systems"
        Bitwig[Bitwig Studio]
        OtherDAWs[Other DAWs]
        FileSystem[File System]
    end
    subgraph "Target Applications"
        CommercialDAW[Commercial DAW]
        OpenSourceDAW[Open Source DAW]
        AudioTools[Audio Tools]
    end
    Library[DAW Project C++ Library]
    Developer[Audio Software Developer]
    Developer --> Library
    CommercialDAW --> Library
    OpenSourceDAW --> Library
    AudioTools --> Library
    Library -.->|reads/writes| FileSystem
    Library -.->|validates| Bitwig
    FileSystem -.->|contains files from| Bitwig
    FileSystem -.->|contains files from| OtherDAWs
```

## Elements & Responsibilities
| ID | Element | Responsibility | Interfaces | Related Requirements |
|----|---------|---------------|-----------|----------------------|
| ARC-C-001 | Library | Project file I/O | C++/C API | REQ-FILE-001 |
| ARC-C-002 | FileSystem | Storage | N/A | REQ-FILE-001 |
| ARC-C-003 | Bitwig | Reference files | N/A | REQ-FILE-001 |

## Design Decisions Referenced
- ADR-001 Architecture style
- ADR-004 Dual API

## Quality Attribute Impact
Supports compatibility, integration, and data integrity.