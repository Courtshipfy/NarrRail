# Tasks: Conversion Review Contract

**Input**: Design documents from `specs/0055-conversion-review-contract/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Contract/core tests are included because the feature affects review queue data ingestion.

## Phase 1: Setup

- [x] T001 Create Spec Kit feature folder at `specs/0055-conversion-review-contract/`
- [x] T002 Add feature specification and quality checklist in `specs/0055-conversion-review-contract/spec.md` and `checklists/requirements.md`
- [x] T003 Add implementation plan, research, data model, contract, quickstart, and tasks files

## Phase 2: Foundational

- [x] T004 Add canonical contract document in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md`
- [x] T005 Update `.codex/skills/narrrail-story-converter/SKILL.md` to require standardized conversion notes when review items exist
- [x] T006 Update `.codex/skills/narrrail-story-converter/references/conversion-policy.md` with review item vocabulary and JSON guidance

## Phase 3: User Story 1 - Human-readable conversion notes (Priority: P1)

**Goal**: Converter authors have a readable `conversion-notes.md` template.

**Independent Test**: A reviewer can read `Docs/spec/CONVERSION_REVIEW_CONTRACT.md` and produce notes without JSON tooling.

- [x] T007 [US1] Define `conversion-notes.md` sections and item table in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md`
- [x] T008 [US1] Add a Markdown example review item in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md`

## Phase 4: User Story 2 - Machine-readable review metadata (Priority: P2)

**Goal**: Project Review Queue can load `conversion-review.json` from an Import Package.

**Independent Test**: `NarrRailEditor/tests/story-project.test.mjs` passes with a package containing `conversion-review.json`.

- [x] T009 [P] [US2] Add regression test for `conversion-review.json` package ingestion in `NarrRailEditor/tests/story-project.test.mjs`
- [x] T010 [US2] Add `conversion-review` asset classification in `NarrRailEditor/src/core/story-project.js`
- [x] T011 [US2] Parse `conversion-review.json` files inside `normalizeImportPackage()` in `NarrRailEditor/src/core/story-project.js`
- [x] T012 [US2] Preserve `sourceLocation`, `generatedTarget`, and `suggestedAction` on queue items in `NarrRailEditor/src/core/story-project.js`

## Phase 5: User Story 3 - Contract consistency (Priority: P3)

**Goal**: Markdown and JSON use one shared vocabulary.

**Independent Test**: Contract docs explain how JSON fields map to Markdown and Project Review Queue fields.

- [x] T013 [US3] Document JSON-to-Markdown consistency rules in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md`
- [x] T014 [US3] Document queue mapping rules in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md`

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T015 Run quickstart validation checks from `specs/0055-conversion-review-contract/quickstart.md`
- [x] T016 Run `npm test` in `NarrRailEditor`
- [x] T017 Run `npm run build` in `NarrRailEditor`
- [x] T018 Run `git diff --check`

## Dependencies

- T004-T006 before user story implementation.
- T009 before T010-T012 if following test-first.
- T013-T014 after the core vocabulary is settled.
- T015-T018 run last.

## MVP Scope

User Story 1 plus the foundational converter policy updates establish the human-readable review contract. User Story 2 is required before closing #55 because the acceptance criteria require JSON items to load into Project Review Queue.
