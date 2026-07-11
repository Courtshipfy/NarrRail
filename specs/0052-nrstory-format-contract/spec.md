# Feature Specification: Neutral .nrstory Format Contract

**Feature Branch**: `[0052-nrstory-format-contract]`

**Created**: 2026-07-11

**Status**: Draft

**Input**: GitHub issue #52, "Extract neutral .nrstory format spec and consumer compatibility contract"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Reference the neutral story contract (Priority: P1)

As a NarrRail maintainer or contributor, I need one neutral `.nrstory` / GlobalConfig format contract owned by the main repository so future editor, converter, preview, and consumer work all reference the same source of truth.

**Why this priority**: Without this contract, the Unreal plugin continues to appear as the format owner and the repository split will carry ambiguous responsibilities.

**Independent Test**: A contributor can find a neutral format spec in the main repository and confirm it covers schema versions, node semantics, validation expectations, and compatibility rules without needing Unreal-specific docs.

**Acceptance Scenarios**:

1. **Given** a contributor opens the main README, **When** they look for `.nrstory` format documentation, **Then** they can reach the neutral contract instead of an Unreal-first runtime document.
2. **Given** a contributor reviews the neutral contract, **When** they inspect `.nrstory` and GlobalConfig requirements, **Then** the schema version, root fields, node types, field semantics, validation expectations, and import/export rules are present.

---

### User Story 2 - Keep Story Consumer boundaries explicit (Priority: P2)

As a Story Consumer maintainer, I need the main repository to state what a consumer may and may not do with NarrRail formats so a future `NarrRail-Unreal-Plugin` repository can declare compatibility without redefining the format.

**Why this priority**: The UE split depends on the consumer repository becoming downstream of the main format contract.

**Independent Test**: A consumer maintainer can read the compatibility section and list which format versions and capabilities their consumer supports.

**Acceptance Scenarios**:

1. **Given** a Story Consumer maintainer reads the contract, **When** they look for compatibility obligations, **Then** they can identify required declarations for Story Script, GlobalConfig, `.nroutline`, unknown fields, and unsupported capabilities.
2. **Given** a platform-specific implementation has sync, asset, Blueprint, PIE, HUD, or Content path behavior, **When** maintainers document it, **Then** the neutral contract directs those details to the consumer repository or compatibility matrix instead of the format spec.

---

### User Story 3 - Preserve current schema behavior (Priority: P3)

As a Narrative Creator using current stories, I need this extraction to avoid changing existing `.nrstory` behavior so current projects remain compatible.

**Why this priority**: This is a contract extraction, not a migration.

**Independent Test**: The resulting docs state that schema changes are out of scope and no runtime/editor schema implementation changes are introduced.

**Acceptance Scenarios**:

1. **Given** existing `.nrstory` and GlobalConfig files, **When** this feature lands, **Then** their schemaVersion and field semantics remain unchanged.
2. **Given** maintainers discover a desired schema change while writing the contract, **When** the change is outside current behavior, **Then** it is deferred to a separate schema migration or compatibility issue.

### Edge Cases

- Existing docs may still mention Unreal-specific sync behavior; those references must be marked as consumer-specific or redirected away from the neutral contract.
- The old `Docs/02_runtime/SCRIPT_FORMAT.md` path may still be referenced by existing docs; it should remain usable as a legacy/runtime entry while pointing to the neutral source of truth.
- `.nroutline` and legacy `.nrrail` must be positioned correctly: `.nroutline` is part of the main Story Project contract, while `.nrrail` is compatibility-only.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The main repository MUST contain a neutral story format contract for `.nrstory`, GlobalConfig, and Story Consumer compatibility.
- **FR-002**: The contract MUST define current schema version ownership for Story Script, GlobalConfig, and `.nroutline`.
- **FR-003**: The contract MUST describe Story Script root structure, required fields, node types, edge semantics, action semantics, validation expectations, and import/export compatibility rules.
- **FR-004**: The contract MUST describe GlobalConfig identification, variable fields, preset speaker purpose, and how scripts reference global variables.
- **FR-005**: The contract MUST keep platform-specific sync, asset, Blueprint, PIE, HUD, and Content path behavior out of the neutral format semantics.
- **FR-006**: The contract MUST include a Story Consumer compatibility section that future consumers can reference.
- **FR-007**: Existing main-repo docs MUST link to the neutral contract without presenting Unreal as the primary workflow.
- **FR-008**: This feature MUST NOT change the actual runtime/editor schema behavior.

### Key Entities *(include if feature involves data)*

- **Story Script**: A `.nrstory` file containing `meta`, `nodes`, and `edges`.
- **GlobalConfig**: A `.nrstory` file identified by `meta.configType: GlobalConfig`, containing project-level variables and preset speakers.
- **Story Outline**: A `.nroutline` file that composes multiple Story Scripts into a Story Project flow.
- **Story Consumer**: A downstream implementation that reads NarrRail story files and declares supported versions and capabilities.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A contributor can identify the neutral format contract from the README in under 2 minutes.
- **SC-002**: The neutral contract includes all issue-required topics: `.nrstory` schema version, GlobalConfig schema version, node types, field semantics, validation expectations, import/export rules, and Story Consumer compatibility.
- **SC-003**: Spec Kit integration status reports Codex as the only installed integration.
- **SC-004**: The feature produces no runtime/editor schema changes.

## Assumptions

- GitHub issue #52 is the source of truth for scope.
- Current schema version remains `1`.
- `Docs/02_runtime/SCRIPT_FORMAT.md` should remain as a legacy/runtime-facing entry because existing documentation references it.
- Consumer-specific UE details will be migrated or rewritten in later issues #51, #53, and #54.
