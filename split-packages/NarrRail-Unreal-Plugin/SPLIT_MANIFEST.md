# Split Manifest: NarrRail-Unreal-Plugin

## Package Identity

- Package name: `NarrRail-Unreal-Plugin`
- Repository URL: <https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>
- Prepared from: `Courtshipfy/NarrRail`
- Prepared date: 2026-07-11
- Related issue: `#54 Create NarrRail-Unreal-Plugin repository split package`
- Strategy: clean copy

This package is the initial repository shape for the Unreal Story Consumer repository. It is not a history-preserving extraction. The original file history remains in the main NarrRail repository.

## Story Consumer Boundary

`NarrRail-Unreal-Plugin` is a Story Consumer. It reads story files authored by the main NarrRail product and declares compatibility with the main repository's neutral contracts.

Main repository ownership:

- `.nrstory` Story Script contract
- GlobalConfig contract
- `.nroutline` Story Outline contract
- schema migration and compatibility policy

Main neutral format contract:

<https://github.com/Courtshipfy/NarrRail/blob/main/Docs/spec/NRSTORY_FORMAT.md>

## Source-To-Target Mapping

| Source path in main repo | Target path in package | Purpose |
| --- | --- | --- |
| `NarrRail/` | `NarrRail/` | Unreal plugin source, descriptor, resources, and plugin content |
| `NarrRailUEHost/` | `NarrRailUEHost/` | UE sample/host project |
| `Docs/01_architecture/TECH_ARCHITECTURE.md` | `Docs/01_architecture/TECH_ARCHITECTURE.md` | UE consumer architecture |
| `Docs/02_runtime/DEBUGGER_GUIDE.md` | `Docs/02_runtime/DEBUGGER_GUIDE.md` | Debugger, HUD, and PIE guidance |
| `Docs/03_ui_blueprint/` | `Docs/03_ui_blueprint/` | Blueprint and UI integration docs |
| `Docs/04_narrrail_ue_host/` | `Docs/04_narrrail_ue_host/` | Host setup, sync, compatibility, and asset sync docs |

## Exclusions

The clean copy intentionally excludes:

- `.git/`
- `.DS_Store`
- Unreal generated outputs: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`
- Local IDE state such as `.vs/`, `.vscode/`, `.idea/`
- Main authoring-product code such as `NarrRailEditor/`
- Main authoring-product planning docs that are not UE consumer-owned
- Main Spec Kit artifacts except this manifest's source issue/spec references

The package includes tracked sample content from `NarrRail/Content/` and `NarrRailUEHost/Content/` because those files are part of the current UE plugin/sample host shape.

## Follow-Up Steps

1. Verify `NarrRailUEHost/NarrRailUEHost.uproject` opens with Unreal Engine `5.7`.
2. Verify `NarrRailUEHost/Build-NarrRailUEHost.cmd.template` works in the standalone repository shape.
3. Update repository URLs in `NarrRail/NarrRail.uplugin` after the new remote exists.
4. Only after the standalone repository is verified, handle main-repo Unreal cleanup in a separate issue.

## Out Of Scope For This Package

- Removing `NarrRail/` or `NarrRailUEHost/` from the main repository.
- Rewriting plugin features.
- Adding `.nroutline` runtime support.
- Changing `.nrstory`, GlobalConfig, or `.nroutline` format semantics.
- Building release packaging or marketplace artifacts.
