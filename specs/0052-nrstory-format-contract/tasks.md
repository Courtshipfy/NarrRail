# Tasks: Neutral .nrstory Format Contract

**Input**: Design documents from `specs/0052-nrstory-format-contract/`

## Phase 1: Setup

- [x] T001 Create Spec Kit feature folder at `specs/0052-nrstory-format-contract/`
- [x] T002 Add feature specification in `specs/0052-nrstory-format-contract/spec.md`
- [x] T003 Add planning artifacts in `specs/0052-nrstory-format-contract/plan.md`, `research.md`, `data-model.md`, `contracts/story-format-contract.md`, and `quickstart.md`

## Phase 2: Foundational

- [x] T004 Remove obsolete non-Codex integration metadata from `.specify/integration.json`
- [x] T005 Remove obsolete non-Codex integration manifest from `.specify/integrations/`
- [x] T006 Update Spec Kit workflow docs in `Docs/agents/spec-kit.md` to describe Codex-only integration
- [x] T007 Update `README.md` Spec Kit file references to describe Codex skills only

## Phase 3: User Story 1 - Reference the neutral story contract (P1)

- [x] T008 [US1] Add neutral contract in `Docs/spec/NRSTORY_FORMAT.md`
- [x] T009 [US1] Link `Docs/02_runtime/SCRIPT_FORMAT.md` to `Docs/spec/NRSTORY_FORMAT.md` as the authoritative neutral contract
- [x] T010 [US1] Ensure `README.md` references `Docs/spec/NRSTORY_FORMAT.md` for `.nrstory` / `.nroutline` format documentation

## Phase 4: User Story 2 - Keep Story Consumer boundaries explicit (P2)

- [x] T011 [US2] Replace UE-specific format compatibility wording in `Docs/02_runtime/SCRIPT_FORMAT.md` with Story Consumer compatibility requirements
- [x] T012 [US2] Document Consumer compatibility obligations in `Docs/spec/NRSTORY_FORMAT.md`

## Phase 5: User Story 3 - Preserve current schema behavior (P3)

- [x] T013 [US3] State no schema behavior changes in `specs/0052-nrstory-format-contract/spec.md`
- [x] T014 [US3] Verify no runtime/editor source files were modified for this issue

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T015 Run text search verification for stale integration references and neutral contract coverage
- [x] T016 Run Spec Kit integration status after Cursor removal
- [x] T017 Run markdown/link-oriented smoke checks from `quickstart.md`

## Dependencies

- T001-T003 complete before implementation.
- T004-T007 can run in parallel with T008-T012.
- T014 depends on all documentation edits being complete.
- T015-T017 run last.

## MVP Scope

User Story 1 is the MVP: a neutral contract exists and is reachable from main repository docs.
