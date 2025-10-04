# Traceability Guidelines

## Purpose

Provide authoritative rules for maintaining end-to-end traceability across requirements, architecture decisions, quality scenarios, and tests while enforcing meaningful (non-inflated) coverage metrics.

## Scope

Applies to all artifact IDs parsed by `scripts/generators/spec_parser.py` and incorporated into `build/traceability.json` via `build_trace_json.py`.

## ID Taxonomy

| Artifact Type | Prefix Pattern | Example |
|---------------|----------------|---------|
| Stakeholder Requirement | `StR-` | `StR-001` |
| Functional Requirement | `REQ-F-` | `REQ-F-001` |
| Non-Functional Requirement | `REQ-NF-` | `REQ-NF-P-001` |
| Architecture Decision Record | `ADR-` | `ADR-004` |
| Quality Attribute Scenario | `QA-SC-` | `QA-SC-003` |
| Test (any layer) | `TEST-` | `TEST-UNIT-MEMORY-001` |

Parser accepts only IDs matching `^(StR|REQ|ARC|ADR|QA|TEST)-[A-Z0-9][A-Z0-9-]*$` (dash is mandatory after the prefix). This prevents false positives like `REQUIRE`.

## Ignored Sources

The parser excludes instructional / template files to avoid placeholder IDs skewing coverage:

- `.github/copilot-instructions.md` (in requirements directory)
- `ADR-template.md`
- `user-story-template.md`
- `architecture-spec.md`
- `requirements-spec.md`

## Link Semantics

Each item stores `references` = forward links to other IDs appearing in its content. The trace builder derives:

- `forward` map: item -> referenced IDs
- `backward` map: referenced ID -> list of items referencing it

### Symmetric Inference Model

A requirement counts as linked to ADR / Scenario / Test if it has EITHER:

1. Forward reference to an ADR / QA-SC-* / TEST-* item, OR
2. Appears in the reverse references of an ADR / QA / TEST item.

This allows downstream artifacts (ADRs, scenarios, tests) to declare driving requirements without forcing forward duplication in the requirement doc.

## Coverage Metrics

Produced in `metrics` section of `traceability.json`:

- `requirement_to_ADR`
- `requirement_to_scenario`
- `requirement_to_test`

Each object includes: `total_requirements`, `requirements_with_link`, `coverage_pct`, per-requirement `details` (forward + reverse refs), and `inference` method.

### Thresholds (Current CI)

| Dimension | Env Var | Threshold |
|-----------|---------|-----------|
| Overall Requirement Linkage | `MIN_REQ_LINKAGE_COVERAGE` | 80% |
| ADR Linkage | `MIN_REQ_ADR_COVERAGE` | 70% |
| Scenario Linkage | `MIN_REQ_SCENARIO_COVERAGE` | 60% |
| Test Linkage | `MIN_REQ_TEST_COVERAGE` | 40% |

## Remediation Workflow

1. Run CI or locally generate artifacts:

   ```bash
   python scripts/generators/spec_parser.py
   python scripts/generators/build_trace_json.py
   ```

2. Inspect failing dimension inside `build/traceability.json` under corresponding `details`.
3. For each uncovered requirement:
   - Confirm it is a real requirement (not placeholder).
   - Add a justified reference:
     - ADR: Update relevant `ADR-XXX` with a rationale including the requirement ID.
     - Scenario: Add the requirement to `relatedRequirements` if the scenario’s stimulus/response legitimately covers it.
     - Test: Annotate test source with the requirement ID in a comment (and ensure the test actually validates it).
4. Rebuild trace JSON and re-run validations.
5. Commit with message referencing IDs updated (e.g., `trace: add REQ-NF-P-001 linkage to QA-SC-001`).

## Adding New Artifacts

- Ensure new IDs follow the taxonomy and increment sequence logically.
- Provide measurable acceptance criteria for requirements & scenarios.
- For new tests, embed `TEST-` ID in a comment near the test definition plus inline `REQ-` references.

## Anti-Inflation Rules

- Do not add requirements to scenarios or ADRs unless there is a clear architectural or behavioral tie.
- Avoid mass-copying IDs across all ADRs / scenarios.
- Each linkage addition should withstand justification review ("Does this artifact materially satisfy or constrain the requirement?").

## Tooling Overview

| Script | Purpose |
|--------|---------|
| `spec_parser.py` | Parse markdown & test sources into structured index. |
| `build_trace_json.py` | Build forward/backward maps and metrics. |
| `trace_unlinked_requirements.py` | Enumerate requirements lacking ADR linkage. |
| `validate-trace-coverage.py` | Enforce thresholds. |

## CI Integration

Workflow job `traceability-coverage`:

1. Downloads generated spec artifacts.
2. Runs unlinked requirements guard (fails if any requirement lacks ADR linkage).
3. Executes `validate-trace-coverage.py` with threshold environment variables.

## Known Limitations / Future Enhancements

- Requirement → Test coverage currently limited by small functional requirement set; consider finer-grained functional requirements as implementation grows.
- Potential extension: export per-requirement linkage status to a markdown summary in reports.
- Add a linter to flag suspicious ID density (possible inflation).

## Review & Ownership

Traceability model custodian: Architecture + QA leads. Changes to thresholds require joint approval.

---

Last Updated: 2025-10-04
