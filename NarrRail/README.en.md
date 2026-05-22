# NarrRail

[中文](./README.md)

NarrRail is an AVG (Adventure / Visual Novel) toolchain. The current primary entry is the **WebEditor authoring side**, backed by the **UE5.7 plugin runtime side**.

---

## 1. WebEditor (Primary Entry)

WebEditor is the most mature entry point for story authoring, structural validation, and preview review.

### 1.1 What you can do

- Graph editing (`Dialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `End`)
- `.nrstory` import/export
- Real-time + manual validation
- Local autosave fallback
- Preview mode with runtime-like execution flow (including exhaustive choice mode)

### 1.2 Quick Start (WebEditor)

Online entry:

- https://narr-rail.courtship.top

Recommended flow:

1. Open WebEditor online, sign in, and enter Script Library
2. Create/open a `.nrstory`, then edit nodes and run validation
3. Use Preview Mode to review execution flow (including branches/end paths)
4. Export `.nrstory` and commit to repository (optional)
5. Import it into UE and validate in PIE

> Local dev entry (`npm run dev`) is intended for editor feature development/debugging, not daily authoring.

### 1.3 WebEditor Docs Hub

- Chinese (default): `Tools/NarrRail.WebEditor/README.md`
- English: `Tools/NarrRail.WebEditor/README.en.md`
- Preview Mode Notes: `Tools/NarrRail.WebEditor/README_READING_MODE.md`
- Deployment Plan: `Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`
- Intro Page Spec: `Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`

---

## 2. UE Plugin (Runtime Side)

The UE plugin is responsible for runtime execution, Blueprint integration, debugging, and asset import.

### 2.1 Current capabilities

- ✅ Runtime Core: state machine, variables, conditions, actions
- ✅ Blueprint API: event system and helper functions
- ✅ YAML Import: drag-and-drop script import into UE Content Browser
- ✅ Debugger: HUD and console commands
- 🚧 Save/Load: planned
- 🚧 YAML Export: in progress

### 2.2 UE-side Integration Flow

1. Export the latest `.nrstory` from WebEditor
2. Open `HostProject/NarrRailHost.uproject` with UE5.7
3. Import/sync script into `UNarrRailStoryAsset`
4. Run `HostProject/Build-HostProject.cmd` if plugin rebuild is needed
5. Validate story execution in PIE with debugger tools

### 2.3 Script Validation (CLI)

```bash
cd Tools/NarrRail.Tooling
dotnet run --project src/NarrRail.Tooling -- validate affinity_demo.nrstory
```

See `Tools/NarrRail.Tooling/README.md`.

---

## 3. Example Scripts

- `Tools/NarrRail.Tooling/test_valid.nrstory` - minimal valid sample
- `Tools/NarrRail.Tooling/affinity_demo.nrstory` - complete affinity branching sample

---

## 4. Core Documentation

- `Docs/02_runtime/SCRIPT_FORMAT.md` - script format specification
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md` - Blueprint quick start
- `Docs/02_runtime/DEBUGGER_GUIDE.md` - debugger guide
- `Docs/01_architecture/TECH_ARCHITECTURE.md` - technical architecture
- `Docs/06_planning/TASK_PLAN.md` - WBS task plan

---

## 5. Repository Structure

- `NarrRail/` plugin source
  - `Source/NarrRail/` runtime module (C++)
  - `Source/NarrRailEditor/` editor module (C++)
- `HostProject/` development host project
- `Tools/NarrRail.WebEditor/` web editor
- `Tools/NarrRail.Tooling/` C# CLI tooling
- `Docs/` project documentation

---

## 6. Language Strategy

- **C++**: runtime and editor core
- **C#**: script processing, validation, CLI tooling
- **Blueprint**: integration layer only (not core execution logic)
