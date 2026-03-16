# NarrRail Host Project Guide

## Goal

Provide a UE5.7 host project in the same repository so you can run, debug, and validate the plugin in editor while keeping a single source of truth for plugin code.

## Layout

- `NarrRail/`: plugin source code (single source of truth)
- `HostProject/`: sample host project for in-editor testing
- `scripts/Setup-HostProject.ps1`: creates a project-to-plugin directory link
- `HostProject/Build-HostProject.ps1`: local build script (same level as `.uproject`)
- `HostProject/Build-HostProject.cmd`: double-click launcher (runs build in CMD)

## One-Time Setup

Run from repository root:

```powershell
.\scripts\Setup-HostProject.ps1
```

The script creates:

- `HostProject/Plugins/NarrRail` -> `NarrRail` (Junction)

This lets the host project load the current plugin source directly without copying files.

## Daily Workflow

1. Run `.\scripts\Setup-HostProject.ps1` (first time or if link is missing).
2. Open `HostProject/NarrRailHost.uproject`.
3. Edit code in `NarrRail/Source/...`.
4. Build from host project script:

```powershell
.\HostProject\Build-HostProject.ps1
```

5. Use PIE or test maps to validate behavior in editor.
6. Update `docs/TASK_PLAN.md` and `docs/TECH_ARCHITECTURE.md` after each completed feature.

## Double-Click Build (Explorer)

- Double-click `HostProject/Build-HostProject.cmd` (defaults to `I:\UE_5.7`).
- The script opens/runs in CMD, executes the PowerShell build script, and keeps the window open with `pause`.
- You can also pass args from terminal, for example:

```cmd
HostProject\Build-HostProject.cmd -Clean
HostProject\Build-HostProject.cmd -EngineRoot "C:\Program Files\Epic Games\UE_5.7"
```

## Build Script

Script path:

- `HostProject/Build-HostProject.ps1`

Default behavior:

- Ensures plugin link exists by running `scripts/Setup-HostProject.ps1`
- Builds `UnrealEditor Win64 Development` with `-Project=HostProject/NarrRailHost.uproject`
- Fails fast if engine path cannot be resolved

Common commands:

```powershell
# Standard build (auto-detect + includes I:\UE_5.7 fallback)
.\HostProject\Build-HostProject.ps1

# Clean intermediates before build
.\HostProject\Build-HostProject.ps1 -Clean

# Explicit engine location
.\HostProject\Build-HostProject.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.7"
```

Notes:

- Build intermediates are local only and ignored by `.gitignore`.
- If needed, set `UE_ENGINE_ROOT` once in your shell profile.

## FAQ

### Recreate the link

```powershell
.\scripts\Setup-HostProject.ps1 -Force
```

### Unreal asks to rebuild modules

This is expected. Rebuild modules, or compile the project target in your IDE first.

### Why not copy plugin files into `HostProject/Plugins`

Copying creates two code sources and easily causes drift. The junction-based setup ensures you always edit and run the same plugin code.
