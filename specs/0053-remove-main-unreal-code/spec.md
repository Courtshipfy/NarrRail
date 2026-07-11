# Feature Specification: Remove Main-Repo Unreal Consumer Code

**Feature Branch**: `[0053-remove-main-unreal-code]`

**Created**: 2026-07-11

**Status**: Draft

**Input**: GitHub issue #53, "Remove or archive Unreal-specific code and docs from the main repo"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Main repository no longer carries active Unreal code (Priority: P1)

As a NarrRail maintainer, I need active Unreal plugin and host project code removed from the main repository so the repository presents itself as the authoring product and neutral format owner.

**Why this priority**: The split repository now exists, so keeping active UE code in the main repository preserves the old product shape.

**Independent Test**: A reviewer can inspect the main repository root and confirm `NarrRail/`, `NarrRailUEHost/`, and the prepared split package copy are gone.

**Acceptance Scenarios**:

1. **Given** a reviewer opens the main repository root, **When** they inspect top-level directories, **Then** active Unreal plugin and host source directories are absent.
2. **Given** a reviewer needs Unreal code, **When** they read main docs, **Then** they are directed to `Courtshipfy/NarrRail-Unreal-Plugin`.

---

### User Story 2 - UE docs become links, not active main docs (Priority: P2)

As a contributor, I need main-repo docs to link to the Unreal consumer repository instead of carrying UE setup, Blueprint, debugger, and host docs locally.

**Why this priority**: Main docs should not imply Unreal is the primary product surface.

**Independent Test**: Searching main docs for old UE doc paths shows no live local links; a single Story Consumer pointer doc provides the new repo URL.

**Acceptance Scenarios**:

1. **Given** a reader looks for Unreal setup, **When** they search the main docs, **Then** they find the Story Consumer pointer to the Unreal repository.
2. **Given** a reader follows main README links, **When** they need UE material, **Then** they land on the external Unreal consumer repository.

---

### User Story 3 - Preserve neutral compatibility notes (Priority: P3)

As a Story Consumer maintainer, I need the main repo to retain neutral compatibility boundaries without retaining consumer-owned implementation docs.

**Why this priority**: The main repository remains format owner and should describe how consumers relate to the format.

**Independent Test**: Neutral format docs remain present and reference Story Consumers without embedding UE implementation instructions.

**Acceptance Scenarios**:

1. **Given** a contributor reads `Docs/spec/NRSTORY_FORMAT.md`, **When** they inspect Story Consumer compatibility, **Then** it describes responsibilities and links to the Unreal consumer repo without owning UE setup details.
2. **Given** a format change is needed by Unreal, **When** docs are followed, **Then** the change starts in the main repository format contract.

### Edge Cases

- Historical specs and ADRs may mention the split as history; those should remain because they explain prior decisions.
- Main-repo research notes may mention old UE coupling as historical context; they do not need to be rewritten unless they create live links.
- The neutral runtime format doc may mention Unreal as an example Story Consumer, but must not include UE setup instructions.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Remove active `NarrRail/` Unreal plugin source from the main repository.
- **FR-002**: Remove active `NarrRailUEHost/` sample/host project from the main repository.
- **FR-003**: Remove the prepared split package copy from the main repository after the external repository is created.
- **FR-004**: Remove UE-owned docs from main docs or replace them with links to `Courtshipfy/NarrRail-Unreal-Plugin`.
- **FR-005**: Add or keep a lightweight main-repo Story Consumer pointer for Unreal.
- **FR-006**: Update README files so Unreal is no longer presented as a local first-class product surface.
- **FR-007**: Preserve `Docs/spec/NRSTORY_FORMAT.md` and neutral Story Consumer compatibility language.
- **FR-008**: Do not change `.nrstory`, GlobalConfig, or `.nroutline` format semantics.

### Key Entities *(include if feature involves data)*

- **Main Repository**: Authoring product and neutral story format owner.
- **Unreal Consumer Repository**: `Courtshipfy/NarrRail-Unreal-Plugin`, owner of UE plugin, host, and UE implementation docs.
- **Story Consumer Pointer**: Lightweight main-repo document that links to the Unreal consumer repo.
- **Neutral Format Contract**: Main-repo `.nrstory`, GlobalConfig, and `.nroutline` contract.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Top-level `NarrRail/`, `NarrRailUEHost/`, and `split-packages/NarrRail-Unreal-Plugin/` paths no longer exist in the main repository.
- **SC-002**: Main README links Unreal users to `https://github.com/Courtshipfy/NarrRail-Unreal-Plugin`.
- **SC-003**: Searches for old local UE doc paths no longer find live README links.
- **SC-004**: `git diff --check` passes after cleanup.

## Assumptions

- `Courtshipfy/NarrRail-Unreal-Plugin` exists and has received the initial split package commit.
- Main-repo cleanup can be committed after the external repository is verified.
- Full UE build validation belongs to the Unreal consumer repository.
