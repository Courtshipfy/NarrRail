# NarrRail

[ä¸­æ–‡](./README.md)

NarrRail is an AVG (Adventure / Visual Novel) toolchain. The current primary entry is the **NarrRailEditor authoring side**, backed by the **UE5.7 plugin runtime side**.

---

## 1. NarrRailEditor (Primary Entry)

NarrRailEditor is the most mature entry point for story authoring, structural validation, and preview review.

### 1.1 What you can do

- Graph editing (`Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `End`)
- `.nrstory` import/export
- Real-time + manual validation
- Local autosave fallback
- Preview mode with runtime-like execution flow (including exhaustive choice mode)

### 1.2 Quick Start (NarrRailEditor)

Online entry:

- https://narr-rail.courtship.top

Recommended flow:

1. Open NarrRailEditor online, sign in, and enter Script Library
2. Create/open a `.nrstory`, then edit nodes and run validation
3. Use Preview Mode to review execution flow (including branches/end paths)
4. Export `.nrstory` and commit to repository (optional)
5. Import it into UE and validate in PIE

> Local dev entry (`npm run dev`) is intended for editor feature development/debugging, not daily authoring.

### 1.3 NarrRailEditor Docs Hub

- Chinese (default): `NarrRailEditor/README.md`
- English: `NarrRailEditor/README.en.md`
- Preview Mode Notes: `NarrRailEditor/README_READING_MODE.md`
- Deployment Plan: `Docs/05_narrrail_editor/NARRRAIL_EDITOR_DEPLOYMENT_PLAN.md`
- Intro Page Spec: `Docs/05_narrrail_editor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`

---

## 2. UE Plugin (Runtime Side)

The UE plugin is responsible for runtime execution, Blueprint integration, debugging, and asset import.

### 2.1 Current capabilities

- âœ?Runtime Core: state machine, variables, conditions, actions
- âœ?Blueprint API: event system and helper functions
- âœ?YAML Import: drag-and-drop script import into UE Content Browser
- âœ?Debugger: HUD and console commands
- ðŸš§ Save/Load: planned
- ðŸš§ YAML Export: in progress

### 2.2 UE-side Integration Flow

1. Export the latest `.nrstory` from NarrRailEditor
2. Open `HostProject/NarrRailHost.uproject` with UE5.7
3. Import/sync script into `UNarrRailStoryAsset`
4. Run `HostProject/Build-HostProject.cmd` if plugin rebuild is needed
5. Validate story execution in PIE with debugger tools

---

## 3. Core Documentation

- `Docs/02_runtime/SCRIPT_FORMAT.md` - script format specification
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md` - Blueprint quick start
- `Docs/02_runtime/DEBUGGER_GUIDE.md` - debugger guide
- `Docs/01_architecture/TECH_ARCHITECTURE.md` - technical architecture
- `Docs/06_planning/TASK_PLAN.md` - WBS task plan

---

## 4. Repository Structure

- `NarrRail/` plugin source
  - `Source/NarrRail/` runtime module (C++)
  - `Source/NarrRailEditor/` editor module (C++)
- `HostProject/` development host project
- `NarrRailEditor/` NarrRailEditor
- `Docs/` project documentation

---

## 5. Language Strategy

- **C++**: runtime and editor core
- **Blueprint**: integration layer only (not core execution logic)
