# Tasks: NarrRail Unreal Plugin Split Package

**Input**: Design documents from `specs/0054-unreal-plugin-split-package/`

## Phase 1: Setup

- [x] T001 Create Spec Kit feature folder at `specs/0054-unreal-plugin-split-package/`
- [x] T002 Add feature specification and quality checklist in `specs/0054-unreal-plugin-split-package/spec.md` and `checklists/requirements.md`
- [x] T003 Add plan, research, data model, contract, and quickstart artifacts in `specs/0054-unreal-plugin-split-package/`

## Phase 2: Foundational

- [x] T004 Create split package root at `split-packages/NarrRail-Unreal-Plugin/`
- [x] T005 Add package `.gitignore` at `split-packages/NarrRail-Unreal-Plugin/.gitignore`
- [x] T006 Copy `NarrRail/` into `split-packages/NarrRail-Unreal-Plugin/NarrRail/` excluding local/generated files
- [x] T007 Copy `NarrRailUEHost/` into `split-packages/NarrRail-Unreal-Plugin/NarrRailUEHost/` excluding local/generated files
- [x] T008 Copy UE-facing docs into `split-packages/NarrRail-Unreal-Plugin/Docs/`

## Phase 3: User Story 1 - Prepare the Unreal repository shape (P1)

- [x] T009 [US1] Add package README at `split-packages/NarrRail-Unreal-Plugin/README.md`
- [x] T010 [US1] Verify package root contains `NarrRail/`, `NarrRailUEHost/`, `Docs/`, `README.md`, `SPLIT_MANIFEST.md`, and `.gitignore`

## Phase 4: User Story 2 - Preserve main-repo format ownership (P2)

- [x] T011 [US2] Add Story Consumer positioning and neutral format links to `split-packages/NarrRail-Unreal-Plugin/README.md`
- [x] T012 [US2] Ensure copied compatibility docs reference main repository neutral format ownership

## Phase 5: User Story 3 - Document clean-copy strategy (P3)

- [x] T013 [US3] Add split manifest at `split-packages/NarrRail-Unreal-Plugin/SPLIT_MANIFEST.md`
- [x] T014 [US3] Document copied source paths, target paths, exclusions, and follow-up cleanup in `split-packages/NarrRail-Unreal-Plugin/SPLIT_MANIFEST.md`

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T015 Run quickstart validation checks from `specs/0054-unreal-plugin-split-package/quickstart.md`
- [x] T016 Run `git diff --check`
- [x] T017 Confirm no files were removed from main `NarrRail/` or `NarrRailUEHost/`
- [x] T018 Create public GitHub repository `Courtshipfy/NarrRail-Unreal-Plugin` and push initial split package commit

## Dependencies

- T001-T003 complete before package implementation.
- T004-T008 must complete before README and manifest validation.
- T009-T012 can run after copied content exists.
- T013-T014 can run after copied content exists.
- T015-T017 run last.

## Parallel Example

After T004:

- T006 and T007 can run in parallel because they copy different directories.
- T008 can run in parallel with T006/T007 because docs copy to a separate target.

## MVP Scope

User Story 1 is the MVP: the prepared package exists with plugin code, host project, and UE docs.
