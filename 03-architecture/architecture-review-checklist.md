---
specType: architecture
standard: "ISO/IEC/IEEE 42010:2011"
phase: "03-architecture"
version: "1.0.0"
author: "Architecture Team"
date: "2025-10-03"
status: "draft"
traceability:
  requirements:
    - "REQ-F-001"
---
# Architecture Review Checklist

This checklist ensures all required artifacts and quality criteria for Phase 03 (Architecture) are met.

## Required Artifacts
- [x] Architecture specification (`03-architecture/architecture-specification.md`)
- [x] At least 3 architectural views documented (`views/context.md`, `views/logical.md`, `views/deployment.md`)
- [x] All ADRs have rationale and link to requirements
- [x] Quality scenarios cover Performance, Availability, Security (`architecture-quality-scenarios.md`)
- [x] Architecture evaluation completed with action items (`architecture-evaluation.md`)
- [x] Traceability matrix shows REQ-* → ARC-C-* linkage (`traceability/architecture-traceability-matrix.md`)

## Quality Criteria
- [x] All views use consistent element IDs (ARC-C-*, ARC-L-*, ARC-D-*)
- [x] All ADRs reference requirements and rationale
- [x] All quality scenarios have measurable metrics and validation methods
- [x] All action items from evaluation are tracked
- [x] Traceability is complete and up to date

## Review Status
- [ ] All reviewers sign off on completeness and quality

## Referenced ADRs
- ADR-001
- ADR-002
- ADR-003
- ADR-004
- ADR-005
- ADR-006
- ADR-007
- ADR-008

---

*Checklist to be updated and signed by architecture reviewers before phase exit.*
