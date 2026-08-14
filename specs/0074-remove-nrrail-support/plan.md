# Implementation Plan: Remove `.nrrail` Support

**Branch**: `codex/remove-nrrail-support` | **Date**: 2026-08-14 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/0074-remove-nrrail-support/spec.md`

## Summary

Make `.nroutline` the sole Story Outline suffix. Remove `.nrrail` from the
GitHub listing adapter, project core asset classification, Script Library,
editor import/export/save paths, regression coverage, and active format
documentation. Keep the existing outline YAML schema and internal `rail`
nomenclature intact.

## Technical Context

**Language/Version**: JavaScript ESM on Node.js 20+; Vue single-file components; Markdown

**Primary Dependencies**: Vue 3, Vite 5, `yaml`

**Storage**: GitHub-backed Story Project assets plus browser-local draft fallback

**Testing**: Node regression suite in `NarrRailEditor/tests/`; Vite production build

**Target Platform**: Browser-based NarrRailEditor and GitHub-backed Story Projects

**Project Type**: Vue authoring application with shell-neutral core modules and adapter routes

**Performance Goals**: Preserve existing project-file listing and outline import/export responsiveness

**Constraints**: No compatibility reader, no automatic rename/migration, no YAML schema change

**Scale/Scope**: One suffix contract across project core, GitHub adapter, editor UI, documentation, and tests

## Constitution Check

- **Story Project Authoring First**: Pass. One suffix reduces ambiguity in the
  Story Project workflow.
- **Neutral Story Format Ownership**: Pass. The active outline suffix contract
  is explicitly updated; the outline YAML schema remains stable.
- **Shell-Neutral Core, Adapter Boundaries**: Pass. Core classification changes
  stay in `src/core/`; browser and GitHub paths change only in their existing
  surfaces and adapters.
- **Spec Before Large Execution**: Pass. This feature has a dedicated Issue #74
  and Spec Kit folder.
- **Reviewable, Testable Changes**: Pass. Project-core classification tests and
  full regression/build validation are required.

## Project Structure

### Documentation (this feature)

```text
specs/0074-remove-nrrail-support/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── contracts/
│   └── outline-file-suffix.md
├── quickstart.md
└── tasks.md
```

### Source Code (repository root)

```text
NarrRailEditor/
├── api/github/file-content.js
├── src/
│   ├── App.vue
│   ├── components/ScriptLibraryPage.vue
│   ├── core/story-project.js
│   └── utils/rail-yaml.js
└── tests/
    ├── outline-format.test.mjs
    ├── story-project.test.mjs
    └── README.md

Docs/
├── 02_runtime/SCRIPT_FORMAT.md
├── 06_planning/OUTLINE_EXTENSION_MIGRATION.md
├── adr/0001-split-unreal-consumer-repository.md
└── spec/NRSTORY_FORMAT.md

README.md
README.en.md
```

**Structure Decision**: Retain the existing split: core decides recognized
project assets; the GitHub adapter lists eligible repository files; Vue
surfaces route input, output, and rename flows; active docs state the public
contract. No new abstraction is needed for a two-suffix removal.

## Complexity Tracking

No constitution violations.
