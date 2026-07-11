# Implementation Plan: NarrRail Unreal Plugin Split Package

**Branch**: `[0054-unreal-plugin-split-package]` | **Date**: 2026-07-11 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/0054-unreal-plugin-split-package/spec.md`

## Summary

Prepare a clean-copy `NarrRail-Unreal-Plugin` split package under `split-packages/` with the current Unreal plugin, sample host project, Unreal-facing documentation, a consumer-positioned README, and a manifest documenting source paths, target paths, and exclusions.

## Technical Context

**Language/Version**: Unreal C++ plugin and Markdown documentation

**Primary Dependencies**: Existing `NarrRail/`, `NarrRailUEHost/`, and `Docs/` source files

**Storage**: Repository filesystem

**Testing**: File inventory checks, text search, whitespace checks

**Target Platform**: Future `NarrRail-Unreal-Plugin` repository maintainers

**Project Type**: Repository packaging / documentation migration

**Performance Goals**: Reviewers can inspect the prepared package root and identify plugin, host, docs, and manifest in under 2 minutes

**Constraints**: No history-preserving extraction; no deletion from main repo; no new Unreal features; no format semantic changes

**Scale/Scope**: One prepared split package directory plus Spec Kit artifacts

## Constitution Check

- **Story Project Authoring First**: Pass. The split isolates UE consumer work from the main authoring product.
- **Neutral Story Format Ownership**: Pass. The package docs link back to the main neutral format contract.
- **Shell-Neutral Core, Adapter Boundaries**: Pass. No shared core implementation changes.
- **Spec Before Large Execution**: Pass. This plan precedes package creation.
- **Reviewable, Testable Changes**: Pass. Manifest and file inventory checks make the package auditable.

## Project Structure

### Documentation (this feature)

```text
specs/0054-unreal-plugin-split-package/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── split-package-manifest.md
├── checklists/
│   └── requirements.md
└── tasks.md
```

### Source Code (repository root)

```text
split-packages/
└── NarrRail-Unreal-Plugin/
    ├── README.md
    ├── SPLIT_MANIFEST.md
    ├── .gitignore
    ├── NarrRail/
    ├── NarrRailUEHost/
    └── Docs/
        ├── 01_architecture/
        ├── 02_runtime/
        ├── 03_ui_blueprint/
        └── 04_narrrail_ue_host/
```

**Structure Decision**: Keep the copied plugin and host directories named `NarrRail/` and `NarrRailUEHost/` so existing plugin descriptors, host project references, and docs remain recognizable during first repository creation.

## Complexity Tracking

No constitution violations.
