# NarrRail

[中文](./README.md)

NarrRail is an AVG (Adventure / Visual Novel) toolchain composed of the **NarrRailEditor authoring app**, the **UE5.7 plugin runtime**, and the **NarrRailUEHost** development host project.

## 1. NarrRailEditor

NarrRailEditor is used for story graph authoring, structural validation, preview review, and `.nrstory` import/export.

### Current Capabilities

- Graph editing: `Dialogue` / `MultiDialogue` / `Choice` / `Jump` / `SetVariable` / `EmitEvent` / `Condition` / `End`
- Multi-branch Condition nodes: each branch maps to a `condition-N` output, with `condition-fallback` for unmatched cases
- Global variables and preset speakers
- `.nrstory` import/export
- Real-time + manual validation
- Local autosave fallback
- Preview mode with runtime-like execution flow

### Quick Start

Online entry:

- https://narr-rail.courtship.top

Recommended flow:

1. Open NarrRailEditor online, sign in, and enter Script Library
2. Create or open a `.nrstory`, then edit nodes and validate
3. Use Preview Mode to review the flow
4. Export `.nrstory` and commit it to the story repository
5. Sync the story repository into UE with `Sync Stories`

Local development (`npm run dev`) is intended for editor feature development and debugging. See `NarrRailEditor/README.md`.

## 2. UE Plugin

The UE plugin handles runtime execution, Blueprint integration, debugging, and story repository sync.

### Current Capabilities

- Runtime Core: state machine, variables, conditions, actions
- Blueprint API: event system and helper functions
- Story Repository Sync: sync `.nrstory` files from a local story Git repository
- Global Config Import: import global variables and preset speakers as `UNarrRailGlobalConfigAsset`
- Debugger: HUD and console commands
- Save/Load: planned

### UE-side Flow

1. Commit exported `.nrstory` files from NarrRailEditor into the local story Git repository
2. Open `NarrRailUEHost/NarrRailUEHost.uproject` with UE5.7
3. Configure `Story Repository Path` in `Project Settings > Plugins > NarrRail`
4. Click `Sync Stories` in the toolbar
5. The plugin runs `git pull --ff-only` by default, then syncs all `.nrstory` files
6. Validate story execution in PIE with debugger tools

Sync rules:

- Default asset root: `/Game/NarrRailStories`
- Actual sync target: `/Game/NarrRailStories/<repository-folder-name>`
- Repository subdirectories are mirrored into UE Content
- Normal story files create/update `UNarrRailStoryAsset`
- `meta.configType: GlobalConfig` files create/update `UNarrRailGlobalConfigAsset`
- Deleted repository files remove the corresponding assets after confirmation

## 3. NarrRailUEHost Plugin Placement

`NarrRailUEHost` needs to resolve the `NarrRail/` plugin source in this repository.

Recommended link:

```text
NarrRailUEHost/Plugins/NarrRail -> ../../NarrRail
```

`NarrRailUEHost/Build-NarrRailUEHost.cmd` automatically checks and creates this Junction. A normal `.lnk` shortcut may not be recognized by Unreal Engine.

## 4. Core Documentation

- `NarrRailEditor/README.md`: editor guide
- `Docs/02_runtime/SCRIPT_FORMAT.md`: script format specification
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`: Blueprint quick start
- `Docs/02_runtime/DEBUGGER_GUIDE.md`: debugger guide
- `Docs/04_narrrail_ue_host/NARRRAIL_UE_HOST.md`: UE host project and story repository sync
- `Docs/01_architecture/TECH_ARCHITECTURE.md`: technical architecture
- `Docs/06_planning/TASK_PLAN.md`: WBS task plan

## 5. Repository Structure

- `NarrRail/`: UE plugin source
  - `Source/NarrRail/`: runtime module (C++)
  - `Source/NarrRailEditor/`: editor module (C++)
- `NarrRailUEHost/`: UE development host project
- `NarrRailEditor/`: Web editor
- `Docs/`: project documentation

## 6. Language Strategy

- **C++**: runtime and editor core
- **Blueprint**: integration layer, not core logic
- **TypeScript / JavaScript / Vue / Svelte**: Web authoring app
