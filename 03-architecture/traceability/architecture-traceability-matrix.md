# Architecture Traceability Matrix

This matrix links requirements (REQ-*) to architecture components (ARC-C-*) and decisions (ADR-*), ensuring traceability per ISO/IEC/IEEE 42010.

| Requirement ID | Architecture Component (ARC-C-*) | ADR | View |
|----------------|-----------------------------------|-----|------|
| REQ-FILE-001   | ARC-C-001, ARC-L-001              | ADR-001, ADR-002, ADR-004 | context, logical |
| REQ-FILE-002   | ARC-L-002, ARC-L-003              | ADR-007 | logical |
| REQ-FILE-003   | ARC-L-003, ARC-L-004              | ADR-005, ADR-006 | logical |
| REQ-NF-P-001   | ARC-L-001, ARC-L-003              | ADR-006 | logical |
| REQ-NF-S-001   | ARC-L-001, ARC-L-002              | ADR-007 | logical |
| REQ-NF-R-002   | ARC-D-001, ARC-D-003              | ADR-008 | deployment |
| NFR-COMPAT-001 | ARC-D-003                         | ADR-008 | deployment |
| NFR-BUILD-001  | ARC-D-002                         | ADR-008 | deployment |

---

*This matrix is updated as requirements, architecture, and decisions evolve.*
