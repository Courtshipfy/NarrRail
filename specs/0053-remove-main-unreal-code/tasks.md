# Tasks: Remove Main-Repo Unreal Consumer Code

**Input**: Design documents from `specs/0053-remove-main-unreal-code/`

## Phase 1: Setup

- [x] T001 Create Spec Kit feature folder at `specs/0053-remove-main-unreal-code/`
- [x] T002 Add feature specification and quality checklist in `specs/0053-remove-main-unreal-code/spec.md` and `checklists/requirements.md`
- [x] T003 Add implementation plan, quickstart, and tasks in `specs/0053-remove-main-unreal-code/`

## Phase 2: Foundational

- [x] T004 Remove active Unreal plugin source directory `NarrRail/`
- [x] T005 Remove active Unreal host project directory `NarrRailUEHost/`
- [x] T006 Remove prepared split package copy `split-packages/NarrRail-Unreal-Plugin/`
- [x] T007 Remove UE-owned implementation docs from `Docs/01_architecture/TECH_ARCHITECTURE.md`, `Docs/02_runtime/DEBUGGER_GUIDE.md`, `Docs/03_ui_blueprint/`, `Docs/04_narrrail_ue_host/`, and `Docs/06_planning/TASK_PLAN.md`

## Phase 3: User Story 1 - Main repository no longer carries active Unreal code (P1)

- [x] T008 [US1] Update `README.md` to describe Unreal only as an external Story Consumer repository
- [x] T009 [US1] Update `README.en.md` to remove local UE plugin and host workflow sections

## Phase 4: User Story 2 - UE docs become links, not active main docs (P2)

- [x] T010 [US2] Add external consumer pointer doc at `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md`
- [x] T011 [US2] Update docs references so old local UE doc paths are not linked from current README surfaces

## Phase 5: User Story 3 - Preserve neutral compatibility notes (P3)

- [x] T012 [US3] Update `Docs/spec/NRSTORY_FORMAT.md` and `Docs/02_runtime/SCRIPT_FORMAT.md` to link to the external Unreal consumer repo
- [x] T013 [US3] Update `.codex/skills/narrrail-story-converter/SKILL.md` to avoid treating the UE plugin as the local compatibility target

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T014 Run quickstart validation checks from `specs/0053-remove-main-unreal-code/quickstart.md`
- [x] T015 Run `git diff --check`
- [x] T016 Confirm external repo `Courtshipfy/NarrRail-Unreal-Plugin` remains reachable

## Dependencies

- T004-T007 should complete before README and doc link rewrites.
- T008-T013 can proceed after removed paths are known.
- T014-T016 run last.

## MVP Scope

User Story 1 is the MVP: active Unreal source and host project are no longer present in the main repository.
