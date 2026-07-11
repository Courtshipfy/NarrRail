# Implementation Plan: Remove Main-Repo Unreal Consumer Code

**Branch**: `[0053-remove-main-unreal-code]` | **Date**: 2026-07-11 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/0053-remove-main-unreal-code/spec.md`

## Summary

Remove active Unreal plugin, host, split package copy, and UE-owned implementation docs from the main repository after the external `NarrRail-Unreal-Plugin` repository has been created. Replace local UE docs with a lightweight Story Consumer pointer and update README surfaces to reflect the main repository's authoring-product role.

## Technical Context

**Language/Version**: Markdown documentation and repository file layout

**Primary Dependencies**: External repository `https://github.com/Courtshipfy/NarrRail-Unreal-Plugin`

**Storage**: Repository filesystem

**Testing**: File existence checks, link/path search, `git diff --check`

**Target Platform**: Main NarrRail repository contributors

**Project Type**: Repository cleanup / documentation repositioning

**Performance Goals**: A contributor can identify the Unreal consumer repository from the main README in under 2 minutes

**Constraints**: No story format changes; no authoring UI rewrite; no UE feature changes

**Scale/Scope**: Top-level UE source directories, UE docs, README files, converter skill wording, one new Story Consumer pointer doc

## Constitution Check

- **Story Project Authoring First**: Pass. Removes UE consumer code from main product surface.
- **Neutral Story Format Ownership**: Pass. Keeps neutral contracts and points consumers outward.
- **Shell-Neutral Core, Adapter Boundaries**: Pass. No core code changes.
- **Spec Before Large Execution**: Pass. Cleanup plan exists before removal.
- **Reviewable, Testable Changes**: Pass. File checks and search validation prove scope.

## Project Structure

### Documentation (this feature)

```text
specs/0053-remove-main-unreal-code/
├── spec.md
├── plan.md
├── quickstart.md
├── checklists/
│   └── requirements.md
└── tasks.md
```

### Source Code (repository root)

```text
Docs/
├── spec/
│   └── NRSTORY_FORMAT.md
├── story-consumers/
│   └── NARRRAIL_UNREAL_PLUGIN.md
└── 02_runtime/
    └── SCRIPT_FORMAT.md

README.md
README.en.md
.codex/skills/narrrail-story-converter/SKILL.md
```

Removed from main repository:

```text
NarrRail/
NarrRailUEHost/
split-packages/NarrRail-Unreal-Plugin/
Docs/01_architecture/TECH_ARCHITECTURE.md
Docs/02_runtime/DEBUGGER_GUIDE.md
Docs/03_ui_blueprint/
Docs/04_narrrail_ue_host/
Docs/06_planning/TASK_PLAN.md
```

**Structure Decision**: Use `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md` as the single main-repo UE pointer rather than keeping UE implementation docs locally.

## Complexity Tracking

No constitution violations.
