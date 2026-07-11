# Implementation Plan: Neutral .nrstory Format Contract

**Branch**: `[0052-nrstory-format-contract]` | **Date**: 2026-07-11 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/0052-nrstory-format-contract/spec.md`

## Summary

Extract a neutral `.nrstory` / GlobalConfig format contract into the main repository, reposition existing format docs to reference that contract, remove obsolete non-Codex Spec Kit integration records, and keep Unreal-specific behavior in consumer-specific documentation rather than the neutral format spec.

## Technical Context

**Language/Version**: Markdown documentation

**Primary Dependencies**: GitHub Issues, Spec Kit documentation workflow

**Storage**: Repository documentation files

**Testing**: Text search, Spec Kit integration status, markdown review

**Target Platform**: Repository contributors and Story Consumer maintainers

**Project Type**: Documentation / contract extraction

**Performance Goals**: Contributors can find the neutral contract from README in under 2 minutes

**Constraints**: No schema implementation changes; no Unreal code movement; no new consumer repository creation

**Scale/Scope**: Main README, Spec Kit workflow docs, neutral format contract, legacy runtime format entry, feature spec folder

## Constitution Check

- **Story Project Authoring First**: Pass. The neutral contract reinforces authoring-product ownership.
- **Neutral Story Format Ownership**: Pass. This is the primary purpose of the feature.
- **Shell-Neutral Core, Adapter Boundaries**: Pass. No code changes; docs keep platform-specific consumer behavior outside the neutral format.
- **Spec Before Large Execution**: Pass. Feature folder created before implementation.
- **Reviewable, Testable Changes**: Pass. Verification uses text search and Spec Kit integration status.

## Project Structure

### Documentation (this feature)

```text
specs/0052-nrstory-format-contract/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── story-format-contract.md
├── checklists/
│   └── requirements.md
└── tasks.md
```

### Source Code (repository root)

```text
Docs/
├── spec/
│   └── NRSTORY_FORMAT.md
├── 02_runtime/
│   └── SCRIPT_FORMAT.md
└── agents/
    └── spec-kit.md

.specify/
├── integration.json
└── integrations/

README.md
```

**Structure Decision**: Use `Docs/spec/NRSTORY_FORMAT.md` for the neutral contract because it is product-format documentation rather than runtime, editor, or UE-specific documentation. Keep `Docs/02_runtime/SCRIPT_FORMAT.md` as a compatibility entry that points to the neutral contract.

## Complexity Tracking

No constitution violations.
