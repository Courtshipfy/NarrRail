# NarrRail UE Plugin

[中文](../README.md)

NarrRail is the UE5.7 runtime/editor plugin for the NarrRail toolchain. It consumes `.nrstory` files authored in NarrRailEditor and turns them into Unreal assets that can be executed from C++ or Blueprint.

## Current Scope

This plugin is suitable for a first technical preview release. It supports story asset import/sync, runtime session execution, Blueprint-facing session APIs, validation, and debug tools.

## Runtime Features

- `UNarrRailStoryAsset` runtime asset
- `UNarrRailStorySession` execution state machine
- Node execution for `Dialogue`, `MultiDialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `Condition`, and `End`
- Variable container with Bool / Int / Float / String support
- Node enter/exit actions
- Multi-branch Condition routing with `condition-N` and `condition-fallback`
- Runtime delegates for Blueprint/UI integration
- Optional dialogue presenter interface for UI widgets
- Save/load snapshot API for story sessions and GameInstance global variables

## Editor Features

- `.nrstory` import through the editor module
- Story repository sync from a local folder
- Optional `git pull --ff-only` before sync
- Mirrored asset output under `/Game/NarrRailStories/<repository-folder-name>`
- Global config import with `meta.configType: GlobalConfig`
- Automatic StoryAsset to GlobalConfig binding during repository sync
- GameInstance-level global variable state shared by story sessions
- Extra managed asset deletion after confirmation
- Story validation during import/sync
- Debug HUD and console commands

## Recommended Workflow

1. Author stories in NarrRailEditor
2. Export `.nrstory` files
3. Commit them to the local story repository
4. Open `Project Settings > Plugins > NarrRail`
5. Set `Story Repository Path`
6. Click `Sync Stories`
7. Use the generated `UNarrRailStoryAsset` in PIE or gameplay code

## Project Settings

Location:

```text
Project Settings > Plugins > NarrRail
```

Settings:

- `Story Repository Path`: local story repository folder
- `Pull Git Repository Before Sync`: enabled by default; runs `git pull --ff-only` when the folder is a Git work tree
- `Story Asset Root Path (UE Content)`: default `/Game/NarrRailStories`

## Known Limitations

- Save/Load exposes snapshots for project SaveGame integration; automatic slot management is left to the host project
- Blueprint helper functions do not yet cover every editor-side authoring feature
- Automated test coverage is limited

## Documentation

- `Docs/02_runtime/SCRIPT_FORMAT.md`: `.nrstory` format
- `Docs/03_ui_blueprint/BLUEPRINT_QUICKSTART.md`: Blueprint quick start
- `Docs/03_ui_blueprint/UI_INTERFACE_DESIGN.md`: UI integration design
- `Docs/04_narrrail_ue_host/NARRRAIL_UE_HOST.md`: host project and sync workflow
