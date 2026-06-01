# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NarrRail is a UE5.7 plugin for AVG (Adventure/Visual Novel) games, providing dialogue runtime, script import/export, and flow debugging capabilities.

**Language Strategy (Mandatory):**
- Primary: C++ for runtime and editor core functionality
- NarrRailEditor: Vue/Svelte authoring UI for script editing, validation, import/export, and preview
- Integration: Blueprint for business integration layer only (NOT for core logic)

**Critical Constraint:** Core execution logic (state machine, condition evaluation, save/load) MUST be implemented in C++. Blueprint is only for calling exposed APIs and event subscription.

## Repository Structure

```
NarrRail/                      # Plugin root
  Source/
    NarrRail/                  # C++ Runtime module
      Public/Runtime/          # Public runtime headers
      Private/Runtime/         # Runtime implementations
    NarrRailEditor/            # C++ Editor module
      Public/                  # Editor public headers
      Private/                 # Editor implementations
  NarrRail.uplugin            # Plugin descriptor

NarrRailUEHost/                   # Development host project
  NarrRailUEHost.uproject       # UE5.7 host project
  Build-NarrRailUEHost.cmd       # Build script
  Plugins/NarrRail/           # Junction/symlink/copy of ../NarrRail

NarrRailEditor/                # Web authoring editor
  src/                         # Vue/Svelte source
  api/                         # Serverless API routes
  package.json                 # Vite app scripts

Docs/
  01_architecture/TECH_ARCHITECTURE.md
  02_runtime/SCRIPT_FORMAT.md
  05_narrrail_editor/          # NarrRailEditor docs
  06_planning/TASK_PLAN.md
```

## Build Commands

**Build the plugin:**
```bash
cd NarrRailUEHost
./Build-NarrRailUEHost.cmd
```

This script invokes UE5.7's UnrealBuildTool to compile the plugin modules.

Before building, make sure `NarrRailUEHost/Plugins/NarrRail` points to the repository plugin source at `NarrRail/`. Use a directory junction/symlink or copy the plugin directory.

## NarrRailEditor Commands

```bash
cd NarrRailEditor
npm install
npm run dev
npm run build
```

**Open in editor:**
```bash
# Double-click NarrRailUEHost.uproject in Explorer
# Or use UE5.7 launcher to open NarrRailUEHost/NarrRailUEHost.uproject
```

**Generate project files:**
```bash
# Right-click NarrRailUEHost.uproject > "Generate Visual Studio project files"
```

## Architecture Layers

### Runtime Module (NarrRail)

**Domain Layer:**
- `NarrRailStoryAsset.h/cpp`: Story asset definition (nodes, edges, variables)
- `NarrRailStoryTypes.h`: Core types (node types, edge, dialogue, choice, condition, action)

**Execution Layer:**
- `NarrRailStorySession.h/cpp`: State machine executor (Start/Next/Choose/Stop/Pause/Resume)
- Session context: current node, variable snapshot, history tracking

**Validation Layer:**
- `NarrRailStoryValidator.h/cpp`: Asset validation (duplicate IDs, broken links, orphaned nodes)

**Current Implementation Status:**
- ✅ Data model (nodes, edges, dialogue, choices, conditions, actions)
- ✅ Asset class with version migration entry (PostLoad)
- ✅ Session executor with basic flow control
- ✅ Condition evaluation and action execution
- 🚧 Variable system (planned: NR-RUN-003-*)
- 🚧 Blueprint API exposure (planned: NR-RUN-005-*)
- 🚧 Save/load system (planned: NR-RUN-006-*)

### Editor Module (NarrRailEditor)

Currently minimal scaffold. Planned features:
- Custom asset editor with graph visualization
- Property inspector and batch editing
- Validation UI with problem navigation
- PIE debugging bridge (runtime node highlighting, variable watching)

## Key Design Decisions

**ADR-001: C++ Core, Blueprint Integration Layer**
- Rationale: Performance, testability, maintainability, controlled error handling
- Impact: Slower initial development than "full Blueprint", but better long-term quality

**ADR-002: C# for Script Tooling**
- Rationale: Better text processing ecosystem, easier CI integration
- Impact: Need to maintain cross-language data contracts and version compatibility

**Script Format:**
- YAML-based (`.nrstory`)
- Schema version 1 (see `Docs/02_runtime/SCRIPT_FORMAT.md`)
- Supports nodes (Dialogue/Choice/Jump/SetVariable/EmitEvent/Condition/End), variables (Bool/Int/Float/String), and Condition-node controlled branching

## Development Workflow

1. Make changes to `NarrRail/Source/...`
2. Run `NarrRailUEHost/Build-NarrRailUEHost.cmd` to compile
3. Open `NarrRailUEHost/NarrRailUEHost.uproject` in UE5.7 editor to test
4. Iterate

**Testing in PIE (Play In Editor):**
- Create story assets in Content Browser
- Test runtime execution in PIE
- Check logs for validation errors and execution traces

## Code Conventions

**Naming:**
- Classes: `UNarrRail*` (UObject-derived), `FNarrRail*` (structs), `ENarrRail*` (enums)
- Modules: `NarrRail` (runtime), `NarrRailEditor` (editor)

**Error Handling:**
- Use error codes for runtime execution failures
- Log errors with context (node ID, asset path)
- Validation errors must be actionable (include location and fix suggestion)

**Performance Baseline (to be established):**
- 1000-node story load time: TBD
- Condition evaluation: support hot-path caching
- Editor large graph operations: track interaction responsiveness

## Task Tracking

All tasks are tracked in `Docs/06_planning/TASK_PLAN.md` with unique IDs (e.g., `NR-RUN-001-01`).

**Task completion criteria:**
- DoD (Definition of Done) met
- Tests pass
- Evidence submitted (commit/PR, test results, screenshots)

**Current milestone:** M1 - Runtime Core (in progress)

## Important Constraints

**Never:**
- Implement core state machine logic in Blueprint
- Skip validation before runtime execution
- Use destructive git operations without explicit user request
- Commit without running validation checks

**Always:**
- Update `TECH_ARCHITECTURE.md` when completing tasks (section 8/9/10)
- Write C++ unit tests for runtime/editor critical logic
- Maintain script format version compatibility
- Document breaking changes with migration paths

## Documentation

- `README.md`: Quick start and structure overview
- `Docs/01_architecture/TECH_ARCHITECTURE.md`: Detailed architecture, module design, interface contracts
- `Docs/06_planning/TASK_PLAN.md`: WBS with completion tracking and DoD
- `Docs/02_runtime/SCRIPT_FORMAT.md`: YAML script format specification v1
- `NarrRailEditor/README.md`: Web editor development guide
- This file: Development guidance for Claude Code

## Version Information

- Plugin version: 0.1.0 (beta/experimental)
- Target engine: UE5.7
- Script schema version: 1
- Platform: Windows (primary), cross-platform planned
