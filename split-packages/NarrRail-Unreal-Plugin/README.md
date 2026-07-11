# NarrRail-Unreal-Plugin

NarrRail-Unreal-Plugin is the Unreal Engine Story Consumer for NarrRail story projects.

It reads `.nrstory` and GlobalConfig files authored by the main NarrRail product, maps them into Unreal assets, and exposes runtime/editor tooling for PIE validation, Blueprint integration, story repository sync, debug HUD, and gameplay-side execution.

The main NarrRail repository remains the owner of the neutral story format contracts:

- Main repository: <https://github.com/Courtshipfy/NarrRail>
- Neutral format contract: <https://github.com/Courtshipfy/NarrRail/blob/main/Docs/spec/NRSTORY_FORMAT.md>

This repository must declare compatibility with those contracts. It should not add private `.nrstory`, GlobalConfig, or `.nroutline` semantics without a main-repo format issue and migration/compatibility decision.

## Repository Status

This repository was initialized from a clean-copy split package generated from the main NarrRail repository.

Source provenance and copy strategy are documented in [SPLIT_MANIFEST.md](./SPLIT_MANIFEST.md).

## Repository Layout

```text
NarrRail-Unreal-Plugin/
├── NarrRail/        # Unreal plugin source, descriptor, resources, and plugin content
├── NarrRailUEHost/  # UE sample/host project for development and validation
├── Docs/            # Unreal consumer documentation
├── README.md
├── SPLIT_MANIFEST.md
└── .gitignore
```

## Compatibility

Current technical preview:

| Plugin version | Unreal Engine | Story Script `.nrstory` | GlobalConfig `.nrstory` | `.nroutline` | Legacy `.nrrail` |
| --- | --- | --- | --- | --- | --- |
| `0.1.0-beta` | UE `5.7` | `schemaVersion: 1` | `schemaVersion: 1` | Not supported | Not supported |

See [Docs/04_narrrail_ue_host/UNREAL_PLUGIN_COMPATIBILITY.md](./Docs/04_narrrail_ue_host/UNREAL_PLUGIN_COMPATIBILITY.md) for the full matrix and setup guide.

## Quick Start

1. Install or open Unreal Engine `5.7`.
2. Open `NarrRailUEHost/`.
3. Copy the local build script template:

   ```cmd
   copy Build-NarrRailUEHost.cmd.template Build-NarrRailUEHost.cmd
   ```

4. Set your UE path:

   ```cmd
   setx UE57_ROOT "I:\UE_5.7"
   ```

5. Run:

   ```cmd
   Build-NarrRailUEHost.cmd
   ```

6. Open `NarrRailUEHost/NarrRailUEHost.uproject`.
7. Configure `Project Settings > Plugins > NarrRail`.
8. Set `Story Repository Path` and click `Sync Stories`.

## Documentation

- [Compatibility and setup](./Docs/04_narrrail_ue_host/UNREAL_PLUGIN_COMPATIBILITY.md)
- [Host project guide](./Docs/04_narrrail_ue_host/NARRRAIL_UE_HOST.md)
- [Blueprint quick start](./Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md)
- [Debugger guide](./Docs/02_runtime/DEBUGGER_GUIDE.md)
- [UE consumer architecture](./Docs/01_architecture/TECH_ARCHITECTURE.md)

## Format Change Policy

Unreal-side feature work may uncover new story format needs. Those requests should start in the main NarrRail repository:

1. Open or reference a main-repo issue describing the `.nrstory`, GlobalConfig, or `.nroutline` change.
2. Explain the Unreal use case as a Story Consumer requirement.
3. Wait for the main contract, validation, migration, and compatibility decision.
4. Update this repository only after the format change is accepted.

Do not add UE-only fields to neutral story files as private extensions.
