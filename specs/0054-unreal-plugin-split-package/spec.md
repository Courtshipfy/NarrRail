# Feature Specification: NarrRail Unreal Plugin Split Package

**Feature Branch**: `[0054-unreal-plugin-split-package]`

**Created**: 2026-07-11

**Status**: Draft

**Input**: GitHub issue #54, "Create NarrRail-Unreal-Plugin repository split package"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Prepare the Unreal repository shape (Priority: P1)

As a maintainer, I need a prepared `NarrRail-Unreal-Plugin` split package that contains the Unreal plugin, sample host project, and Unreal-facing docs so a separate repository can be created without re-deciding file boundaries.

**Why this priority**: The repository split cannot proceed safely until the target repository shape exists and can be inspected as a clean copy.

**Independent Test**: A reviewer can open the prepared package and find plugin source, host project, and UE documentation without relying on the main repository's authoring-product docs.

**Acceptance Scenarios**:

1. **Given** a reviewer opens the split package, **When** they inspect its root, **Then** it presents itself as `NarrRail-Unreal-Plugin`.
2. **Given** a reviewer inspects the package contents, **When** they look for plugin and host assets, **Then** `NarrRail/` and `NarrRailUEHost/` are present.
3. **Given** a reviewer inspects documentation, **When** they look for UE setup, Blueprint, debugger, sync, and compatibility material, **Then** those documents are present under the package docs.

---

### User Story 2 - Preserve main-repo format ownership (Priority: P2)

As a Story Consumer maintainer, I need the split package to say that the Unreal repository consumes `.nrstory` rather than owning it, so future format changes continue through the main repository.

**Why this priority**: The split is part of the main repository's repositioning as the authoring product and neutral format owner.

**Independent Test**: A reviewer can read the package README and compatibility docs and identify links back to the main repository's neutral format contract.

**Acceptance Scenarios**:

1. **Given** a contributor reads the split package README, **When** they look for format ownership, **Then** it says the main repository owns `.nrstory`, GlobalConfig, and `.nroutline`.
2. **Given** an Unreal-side issue requests a new story field, **When** maintainers follow the split package docs, **Then** the request is routed to the main repository first.

---

### User Story 3 - Document clean-copy strategy (Priority: P3)

As a future release maintainer, I need the copy strategy and source paths documented so the package can be refreshed or verified without guessing.

**Why this priority**: The issue explicitly excludes history-preserving extraction, so the clean-copy strategy must be auditable.

**Independent Test**: A reviewer can compare a manifest against source paths and confirm the package was prepared by clean-copying current main-repo files.

**Acceptance Scenarios**:

1. **Given** a maintainer wants to refresh the split package, **When** they read the manifest, **Then** they can see source paths and target paths.
2. **Given** a reviewer checks the package, **When** they inspect the manifest, **Then** it states that main-repo Unreal file removal is out of scope.

### Edge Cases

- Generated or local Unreal build artifacts must not be intentionally packaged.
- `.DS_Store` and local machine files must not be included.
- Existing main-repo Unreal files must remain in place for later cleanup tickets.
- The split package may contain binary `.uasset` sample content when that content is already tracked as part of the sample host.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The repository MUST include a prepared `NarrRail-Unreal-Plugin` split package.
- **FR-002**: The package MUST include the current Unreal plugin source from `NarrRail/`.
- **FR-003**: The package MUST include the current sample host project from `NarrRailUEHost/`.
- **FR-004**: The package MUST include Unreal-facing setup, Blueprint, debugger, HUD/PIE, Sync Stories, asset sync, and compatibility documentation.
- **FR-005**: The package README MUST present the repository as a Story Consumer.
- **FR-006**: The package MUST reference the main repository neutral format contract instead of redefining `.nrstory`.
- **FR-007**: The package MUST document source paths, target paths, and the clean-copy strategy.
- **FR-008**: This feature MUST NOT remove Unreal files from the main repository.
- **FR-009**: This feature MUST NOT add Unreal plugin features or change story format semantics.

### Key Entities *(include if feature involves data)*

- **Split Package**: The prepared directory that can become `NarrRail-Unreal-Plugin`.
- **Unreal Plugin**: The `NarrRail/` plugin code, resources, and plugin descriptor.
- **Sample Host Project**: The `NarrRailUEHost/` project used to build and test the plugin.
- **Unreal Consumer Docs**: Documentation owned by the future consumer repository.
- **Split Manifest**: A record of copied source paths, target paths, and exclusions.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A reviewer can locate plugin code, host project, compatibility matrix, and setup docs from the split package root in under 2 minutes.
- **SC-002**: The manifest lists every top-level source path copied into the package.
- **SC-003**: The package README states Story Consumer positioning and links to the main neutral format contract.
- **SC-004**: No files are removed from `NarrRail/` or `NarrRailUEHost/` in the main repository.

## Assumptions

- The prepared package will live under `split-packages/NarrRail-Unreal-Plugin/` until a real repository is created.
- This issue prepares the package only; creating or pushing a new remote repository can happen after review.
- UE build validation may require the user's local Unreal Engine installation and is not required for docs/package preparation.
