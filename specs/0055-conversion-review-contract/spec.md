# Feature Specification: Conversion Review Contract

**Feature Branch**: `0055-conversion-review-contract`

**Created**: 2026-07-11

**Status**: Draft

**Input**: GitHub Issue #55: Standardize `conversion-notes.md` and optional `conversion-review.json` for conversion ambiguity, omission, and human-review items.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Human-readable conversion notes (Priority: P1)

A Narrative Creator receives converted story files and a `conversion-notes.md` that clearly explains what was omitted, inferred, ambiguous, or still needs review.

**Why this priority**: The human-readable notes are the minimum viable review surface; they keep AI conversion output from becoming authoritative without inspection.

**Independent Test**: A reviewer can open only `conversion-notes.md` and understand every review item without running NarrRail tooling.

**Acceptance Scenarios**:

1. **Given** a converted manuscript has uncertain speakers, **When** the converter writes `conversion-notes.md`, **Then** the notes include source location, issue type, severity, generated target, and suggested action.
2. **Given** source text is omitted from `.nrstory`, **When** the reviewer reads `conversion-notes.md`, **Then** the omitted content and reason are visible in a dedicated review section.

---

### User Story 2 - Machine-readable review metadata (Priority: P2)

Project Review Queue can load optional `conversion-review.json` from an Import Package and show the same review items as structured metadata.

**Why this priority**: The JSON contract lets conversion output feed future Import Package Review and Project Review Queue UI without scraping Markdown.

**Independent Test**: An Import Package containing `conversion-review.json` but no manually supplied review items can produce Project Review Queue review items.

**Acceptance Scenarios**:

1. **Given** an Import Package contains `conversion-review.json`, **When** Project Review Queue is built, **Then** JSON review items appear with severity, category, source location, target, and suggested action.
2. **Given** a review item points at a generated `.nrstory` node, **When** the queue item is normalized, **Then** the target retains `assetPath` and `nodeId`.

---

### User Story 3 - Contract consistency between Markdown and JSON (Priority: P3)

Converter authors and future UI implementers share one vocabulary for issue types, severities, source locations, generated targets, and suggested actions.

**Why this priority**: Shared vocabulary prevents the converter, Review Queue, and future Import Package Review UI from drifting.

**Independent Test**: The contract document describes how every JSON item should be represented in Markdown notes.

**Acceptance Scenarios**:

1. **Given** a converter emits both files, **When** a review item appears in JSON, **Then** the Markdown notes include a corresponding human-readable entry.
2. **Given** a review item cannot map to generated output, **When** it is written, **Then** it still maps to a source-manuscript location and suggested action.

### Edge Cases

- A review item may reference only the source manuscript, with no generated target.
- A review item may reference `.nrstory`, `.nroutline`, GlobalConfig, conversion profile, or package-level output.
- A converter may emit Markdown notes without JSON; JSON is optional.
- A converter may emit JSON without stable generated IDs during early exploration; such items must still include source location and suggested action.
- Invalid JSON should create a parse/error queue item rather than silently disappearing.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The repository MUST define a `conversion-notes.md` Markdown template for human review.
- **FR-002**: The repository MUST define an optional `conversion-review.json` contract.
- **FR-003**: Each review item MUST support source location, issue type, generated target, severity, message, and suggested action.
- **FR-004**: Review items MUST be able to target `.nrstory` nodes/edges, `.nroutline` nodes/edges, GlobalConfig fields, conversion profile fields, package-level output, or source-manuscript locations.
- **FR-005**: Markdown notes and JSON review metadata MUST use the same item vocabulary.
- **FR-006**: Project Review Queue MUST be able to load review items from `conversion-review.json` inside an Import Package.
- **FR-007**: The converter skill MUST direct agents to emit standardized notes and, when useful, optional JSON review metadata.
- **FR-008**: This feature MUST NOT change `.nrstory`, GlobalConfig, or `.nroutline` semantics.

### Key Entities *(include if feature involves data)*

- **Conversion Notes**: Human-readable Markdown review file for converter output.
- **Conversion Review Document**: Optional JSON document carrying structured review metadata.
- **Conversion Review Item**: One ambiguity, omission, validation concern, or human-review request.
- **Source Location**: A manuscript/file/table/line reference that explains where the issue came from.
- **Generated Target**: The generated NarrRail asset, node, edge, field, or package output related to the issue.
- **Suggested Action**: Human next step, such as confirm, rewrite, split node, add branch, or ignore.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A contributor can create a valid `conversion-notes.md` from the template without reading converter implementation code.
- **SC-002**: A sample Import Package with `conversion-review.json` produces at least one Project Review Queue item in regression tests.
- **SC-003**: Every documented JSON review item field has a Markdown representation rule.
- **SC-004**: Existing story format tests still pass, proving the neutral story formats were not changed.

## Assumptions

- `conversion-review.json` is optional for now; `conversion-notes.md` remains the required human-readable artifact when review items exist.
- This issue defines the contract and the Project Review Queue load seam, not the full Import Package Review UI.
- Existing `reviewItems` on Import Packages remain supported and are merged with items parsed from `conversion-review.json`.
