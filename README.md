# NarrRail

UE5.7 plugin for AVG (Adventure/Visual Novel) games, providing dialogue runtime, script import/export, and flow debugging capabilities.

## Features

- ✅ **Runtime Core**: State machine, variables, conditions, actions
- ✅ **Blueprint API**: Full event system and helper functions
- ✅ **Script Validation**: C# CLI tool for YAML script validation
- ✅ **YAML Import**: Drag-and-drop YAML scripts into UE Content Browser
- ✅ **Debugger**: Screen HUD and console commands
- 🚧 **Save/Load**: Planned
- 🚧 **Visual Editor**: Planned
- 🚧 **YAML Export**: Planned

## Structure

- `NarrRail/`: Plugin source
  - `Source/NarrRail/`: Runtime module (C++)
  - `Source/NarrRailEditor/`: Editor module (C++)
- `HostProject/`: Development host project
- `Tools/NarrRail.Tooling/`: C# CLI tools for script validation
- `docs/`: Documentation

## Quick Start

### 1. Script Validation (CLI)

```bash
cd Tools/NarrRail.Tooling
dotnet run --project src/NarrRail.Tooling -- validate affinity_demo.narrrail.yaml
```

See `Tools/NarrRail.Tooling/README.md` for CLI usage.

### 2. Import YAML Script to UE

1. Open UE Editor with `HostProject/NarrRailHost.uproject`
2. Drag `Tools/NarrRail.Tooling/affinity_demo.narrrail.yaml` into Content Browser
3. A `UNarrRailStoryAsset` will be automatically created

### 3. Blueprint Usage

See `docs/BLUEPRINT_QUICKSTART.md` for creating dialogues in Blueprint.

### 4. Debugging

See `docs/DEBUGGER_GUIDE.md` for runtime debugging.

## Example Scripts

- `Tools/NarrRail.Tooling/test_valid.narrrail.yaml` - Minimal valid script
- `Tools/NarrRail.Tooling/affinity_demo.narrrail.yaml` - Complete affinity system demo

## Documentation

- `docs/SCRIPT_FORMAT.md` - YAML script format specification
- `docs/BLUEPRINT_QUICKSTART.md` - Blueprint quick start guide
- `docs/DEBUGGER_GUIDE.md` - Debugger usage guide
- `docs/TECH_ARCHITECTURE.md` - Technical architecture
- `docs/TASK_PLAN.md` - WBS task checklist

## Install (Project Plugin)

1. Copy the `NarrRail` folder into your UE project at `Plugins/NarrRail`.
2. Generate project files.
3. Build the project and enable the plugin in the editor.

## Develop With Host Project

1. Open `HostProject/NarrRailHost.uproject` in UE5.7.
2. Run `HostProject/Build-HostProject.cmd` to compile.
3. Iterate on `NarrRail/Source/...` and rebuild.

## Language Strategy

- **C++**: Runtime and editor core functionality
- **C#**: Script processing, validation, CLI tools
- **Blueprint**: Business integration layer only (NOT for core logic)
