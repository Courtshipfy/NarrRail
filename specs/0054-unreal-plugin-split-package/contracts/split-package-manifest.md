# Contract: Split Package Manifest

The split package must contain `SPLIT_MANIFEST.md` at its root.

## Required Sections

- Package identity
- Story Consumer positioning
- Source repository
- Copy strategy
- Source-to-target mapping
- Exclusions
- Main-repo ownership boundaries
- Follow-up steps

## Required Source Mappings

| Source | Target | Purpose |
| --- | --- | --- |
| `NarrRail/` | `NarrRail/` | Unreal plugin source, descriptor, resources, and content |
| `NarrRailUEHost/` | `NarrRailUEHost/` | Sample host project |
| `Docs/01_architecture/TECH_ARCHITECTURE.md` | `Docs/01_architecture/TECH_ARCHITECTURE.md` | UE consumer architecture |
| `Docs/02_runtime/DEBUGGER_GUIDE.md` | `Docs/02_runtime/DEBUGGER_GUIDE.md` | Debugger, HUD, and PIE guidance |
| `Docs/03_ui_blueprint/` | `Docs/03_ui_blueprint/` | Blueprint and UI integration docs |
| `Docs/04_narrrail_ue_host/` | `Docs/04_narrrail_ue_host/` | Host setup, sync, compatibility docs |

## Required Exclusion Notes

- `.git/`
- `.DS_Store`
- Unreal generated build outputs such as `Binaries/`, `Intermediate/`, `Saved/`, and `DerivedDataCache/`
- Main authoring-product code and docs not owned by the UE consumer

## Required Boundary Statement

The manifest must state that the main NarrRail repository remains the owner of the neutral `.nrstory`, GlobalConfig, and `.nroutline` contracts.
