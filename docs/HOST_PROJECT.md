# NarrRail Host Project Guide

## Goal

Provide a UE5.7 host project in the same repository so you can run, debug, and validate the plugin in editor while keeping a single source of truth for plugin code.

## Layout

- `NarrRail/`: plugin source code (single source of truth)
- `HostProject/`: sample host project for in-editor testing
- `HostProject/Build-HostProject.cmd`: double-click build entry (CMD only)

## One-Time Setup

No separate setup script is required.

`Build-HostProject.cmd` will auto-check and create:

- `HostProject/Plugins/NarrRail` -> `NarrRail` (Junction)

## Daily Workflow

1. Open `HostProject/NarrRailHost.uproject`.
2. Edit code in `NarrRail/Source/...`.
3. Double-click `HostProject/Build-HostProject.cmd` to compile.
4. Use PIE or test maps to validate behavior in editor.

## Build Entry

- `HostProject/Build-HostProject.cmd`

Default behavior:

- Uses engine root: `I:\UE_5.7`
- Builds: `UnrealEditor Win64 Development`
- Project: `HostProject/NarrRailHost.uproject`
- Auto-creates plugin junction if missing

## FAQ

### Why CMD only

To avoid any dependency on missing `.ps1` files and keep build flow directly runnable from Explorer.
