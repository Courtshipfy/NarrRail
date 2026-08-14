# Tasks: Remove `.nrrail` Support

**Input**: Design documents from `specs/0074-remove-nrrail-support/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Project-core regression coverage is required because this feature
changes Story Project asset classification and import/export behavior.

## Phase 1: Setup

- [x] T001 Create the Issue #74 Spec Kit artifacts in `specs/0074-remove-nrrail-support/`

## Phase 2: Foundational

- [x] T002 Add `.nrrail` rejection coverage in `NarrRailEditor/tests/story-project.test.mjs`
- [x] T003 Remove `.nrrail` classification and legacy asset metadata from `NarrRailEditor/src/core/story-project.js`
- [x] T004 Restrict GitHub repository listing to `.nrstory` and `.nroutline` in `NarrRailEditor/api/github/file-content.js`

## Phase 3: User Story 1 - Work with one outline suffix (Priority: P1)

**Goal**: Creators create, open, save, import, export, and rename Story Outlines
as `.nroutline` files.

**Independent Test**: An empty project produces `Stories/main_story.nroutline`,
and the editor can import/export `.nroutline` files.

- [x] T005 [US1] Route `.nroutline` library entries, creation, and rename behavior in `NarrRailEditor/src/components/ScriptLibraryPage.vue`
- [x] T006 [US1] Route `.nroutline` imports and GitHub outline saves in `NarrRailEditor/src/App.vue`
- [x] T007 [US1] Export Outline downloads with the `.nroutline` suffix in `NarrRailEditor/src/utils/rail-yaml.js`

## Phase 4: User Story 2 - Reject retired outline files (Priority: P2)

**Goal**: Retired `.nrrail` files cannot enter active authoring paths.

**Independent Test**: Source and adapter paths contain no `.nrrail` recognition,
and the project-core regression suite classifies it as `unknown`.

- [x] T008 [US2] Remove retired-suffix test terminology from `NarrRailEditor/tests/outline-format.test.mjs` and update `NarrRailEditor/tests/README.md`

## Phase 5: User Story 3 - Read the current format contract (Priority: P3)

**Goal**: Active documentation declares `.nroutline` as the only supported
outline suffix.

**Independent Test**: Active README and format references contain no claim that
`.nrrail` remains compatible.

- [x] T009 [P] [US3] Update active README suffix guidance in `README.md` and `README.en.md`
- [x] T010 [P] [US3] Update the neutral and runtime contracts in `Docs/spec/NRSTORY_FORMAT.md` and `Docs/02_runtime/SCRIPT_FORMAT.md`
- [x] T011 [P] [US3] Replace compatibility migration guidance in `Docs/06_planning/OUTLINE_EXTENSION_MIGRATION.md`
- [x] T012 [P] [US3] Correct the active format reference in `Docs/adr/0001-split-unreal-consumer-repository.md`

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T013 Run `npm test` in `NarrRailEditor`
- [x] T014 Run `npm run build` in `NarrRailEditor`
- [x] T015 Verify no `.nrrail` recognition remains in `NarrRailEditor/src` or `NarrRailEditor/api`
- [ ] T016 Run `git diff --check` and review the implementation against `specs/0074-remove-nrrail-support/spec.md`

## Dependencies

- T002 must precede T003 so the retired suffix is first covered by a failing regression.
- T003 and T004 must finish before user-story verification.
- T005-T007 depend on T004 only for end-to-end suffix consistency.
- T008 can run after T003.
- T009-T012 can run in parallel with the implementation phases.
- T013-T016 run last.

## MVP Scope

User Story 1 plus the foundational core and adapter changes establish the
`.nroutline`-only authoring path. User Stories 2 and 3 complete the requested
removal by eliminating retired-file entry points and contradictory active docs.
