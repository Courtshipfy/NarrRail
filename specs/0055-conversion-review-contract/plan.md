# Implementation Plan: Conversion Review Contract

**Branch**: `0055-conversion-review-contract` | **Date**: 2026-07-11 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/0055-conversion-review-contract/spec.md`

## Summary

Define a shared review-note contract for Script Conversion and future Import Package Review. The implementation adds a canonical contract document, updates the converter skill to use it, and teaches the existing Story Project review core to load optional `conversion-review.json` items from Import Packages.

## Technical Context

**Language/Version**: JavaScript ESM on Node.js 20+ for editor core tests; Markdown/JSON for contracts

**Primary Dependencies**: Existing `yaml` package; no new runtime dependencies

**Storage**: GitHub-backed Story Project files and Import Package file payloads

**Testing**: `npm test` in `NarrRailEditor`

**Target Platform**: NarrRailEditor shared core and project-level Codex converter skill

**Project Type**: Single repository with a Vue editor app and shell-neutral core modules

**Performance Goals**: Parse small review metadata files during Project Review Queue construction without noticeable user delay

**Constraints**: No `.nrstory`, GlobalConfig, or `.nroutline` schema changes; no full Import Package Review UI

**Scale/Scope**: Contract and loader seam for conversion notes/review metadata

## Constitution Check

- **Story Project Authoring First**: Pass. The work strengthens review and conversion handoff inside Story Project.
- **Neutral Story Format Ownership**: Pass. No neutral story format semantics are changed.
- **Shell-Neutral Core, Adapter Boundaries**: Pass. JSON loading belongs in `NarrRailEditor/src/core/story-project.js`, not Vue UI.
- **Spec Before Large Execution**: Pass. This folder records spec, plan, data model, contracts, tasks, and validation.
- **Reviewable, Testable Changes**: Pass. Contract tests are added at the shared core seam.

## Project Structure

### Documentation (this feature)

```text
specs/0055-conversion-review-contract/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── conversion-review-json.md
└── tasks.md
```

### Source Code (repository root)

```text
Docs/spec/
└── CONVERSION_REVIEW_CONTRACT.md

.codex/skills/narrrail-story-converter/
├── SKILL.md
└── references/conversion-policy.md

NarrRailEditor/src/core/
└── story-project.js

NarrRailEditor/tests/
└── story-project.test.mjs
```

**Structure Decision**: Keep the contract in `Docs/spec/` and the loader in the existing shell-neutral Story Project core. Avoid new packages or UI work.

## Complexity Tracking

No constitution violations.
