# Feature Specification: Remove `.nrrail` Support

**Feature Branch**: `codex/remove-nrrail-support`

**Created**: 2026-08-14

**Status**: Ready for implementation

**Input**: GitHub Issue #74: Remove `.nrrail` support from NarrRail authoring.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Work with one outline suffix (Priority: P1)

A Narrative Creator creates, opens, saves, imports, and exports Story Outlines using
only the `.nroutline` suffix.

**Why this priority**: A single supported suffix makes the Story Project contract,
editor behavior, and handoff to Story Consumers unambiguous.

**Independent Test**: Create or export an outline, then verify it is named
`*.nroutline`; select a `.nroutline` file for import and verify it opens as an
outline.

**Acceptance Scenarios**:

1. **Given** a Story Project has no outline, **When** the creator opens the
   outline workspace, **Then** the created outline path is
   `Stories/main_story.nroutline`.
2. **Given** an outline is exported, **When** its download is created, **Then**
   its file name ends with `.nroutline`.
3. **Given** a creator selects a `.nroutline` file, **When** it is imported,
   **Then** NarrRail opens it as a Story Outline.

---

### User Story 2 - Reject retired outline files (Priority: P2)

A Narrative Creator cannot accidentally treat a `.nrrail` file as a current Story
Outline in the project library, import path, or project snapshot.

**Why this priority**: Direct removal must make retired files visibly unsupported,
rather than silently preserving an obsolete compatibility route.

**Independent Test**: Classify a `.nrrail` project file and verify that it is not
an outline; verify the editor file chooser does not advertise `.nrrail`.

**Acceptance Scenarios**:

1. **Given** a Story Project scan encounters `Stories/main_story.nrrail`,
   **When** NarrRail classifies project assets, **Then** that file is not an
   outline asset.
2. **Given** a creator opens the editor import control, **When** supported file
   types are presented, **Then** `.nrrail` is absent.
3. **Given** a Story Project has an outline, **When** the creator renames it,
   **Then** the resulting name ends with `.nroutline`.

---

### User Story 3 - Read the current format contract (Priority: P3)

A Story Consumer implementer or Narrative Creator can determine from active
documentation that `.nroutline` is the sole supported outline suffix.

**Why this priority**: Format decisions must be explicit before authoring and
consumer implementations can rely on them.

**Independent Test**: Read the active README and neutral format contract; neither
claims that `.nrrail` can be read, edited, or migrated by NarrRail.

**Acceptance Scenarios**:

1. **Given** a contributor reads the neutral format contract, **When** they look
   up Story Outline files, **Then** it lists `.nroutline` without a legacy
   outline suffix.
2. **Given** a contributor reads active migration guidance, **When** they look
   for compatibility behavior, **Then** it states that `.nrrail` is unsupported
   and must be renamed outside NarrRail before use.

### Edge Cases

- A `.nrrail` file remains in a GitHub-backed Story Project; it is not listed as
  a supported Story Outline and does not become an outline in a project snapshot.
- A caller attempts to classify a `.nrrail` path directly; classification returns
  an unknown asset instead of an outline.
- Existing outline YAML continues to use the stable `meta.railId` field; removing
  the file suffix does not change its YAML structure.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: NarrRail MUST recognize `.nroutline` as the only Story Outline file suffix.
- **FR-002**: NarrRail MUST create, rename, save, import, and export Story
  Outlines with the `.nroutline` suffix.
- **FR-003**: NarrRail MUST NOT classify `.nrrail` files as Story Outlines.
- **FR-004**: NarrRail MUST NOT present `.nrrail` as an importable, writable, or
  editable Story Outline path.
- **FR-005**: Active product and format documentation MUST state that
  `.nroutline` is the only supported outline suffix.
- **FR-006**: Regression coverage MUST verify both `.nroutline` support and
  `.nrrail` rejection at the project core seam.
- **FR-007**: Removing `.nrrail` support MUST NOT change Story Outline YAML
  fields or the `.nrstory` and GlobalConfig contracts.

### Key Entities

- **Story Outline**: The project-level story orchestration asset stored as a
  `.nroutline` file.
- **Retired Outline File**: A file with the `.nrrail` suffix. It is not a
  recognized NarrRail Story Outline.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Every Story Outline path produced by the authoring product ends
  with `.nroutline`.
- **SC-002**: Project asset classification returns `unknown` for 100% of
  `.nrrail` path inputs.
- **SC-003**: The editor import control advertises exactly `.nrstory` and
  `.nroutline` Story Project content file suffixes.
- **SC-004**: The full editor regression suite and production build complete
  successfully after the compatibility removal.

## Assumptions

- Existing `.nrrail` files are deliberately unsupported; users must rename them
  to `.nroutline` outside NarrRail before opening them.
- This feature removes file-suffix support only. Internal `rail` terminology and
  the `meta.railId` schema field remain unchanged.
- Historical specs and research records may describe former compatibility but do
  not define the active product contract.
