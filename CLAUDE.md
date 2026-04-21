# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NarrRail is a UE5.7 plugin for AVG (Adventure/Visual Novel) games, providing dialogue runtime, script import/export, and flow debugging capabilities.

**Language Strategy (Mandatory):**
- Primary: C++ for runtime and editor core functionality
- Tooling: C# for script processing, validation, and CLI tools (planned)
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

HostProject/                   # Development host project
  NarrRailHost.uproject       # UE5.7 host project
  Build-HostProject.cmd       # Build script
  Plugins/NarrRail/           # Junction to ../NarrRail (auto-created)

docs/
  TECH_ARCHITECTURE.md        # Technical architecture and design decisions
  TASK_PLAN.md               # WBS task checklist with completion tracking
  SCRIPT_FORMAT.md           # YAML script format specification
```

## Build Commands

**Build the plugin:**
```bash
cd HostProject
./Build-HostProject.cmd
```

This script:
- Creates a junction from `HostProject/Plugins/NarrRail` to the plugin source
- Invokes UE5.7's UnrealBuildTool to compile the plugin modules
- Requires UE5.7 installed at `I:\UE_5.7`

**Open in editor:**
```bash
# Double-click NarrRailHost.uproject in Explorer
# Or use UE5.7 launcher to open HostProject/NarrRailHost.uproject
```

**Generate project files:**
```bash
# Right-click NarrRailHost.uproject > "Generate Visual Studio project files"
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

### Tooling (Planned)

C# CLI tools for:
- Script validation (`narrrail validate <script>`)
- Import/export (`narrrail import/export`)
- Round-trip consistency checking
- Localization text extraction

## Key Design Decisions

**ADR-001: C++ Core, Blueprint Integration Layer**
- Rationale: Performance, testability, maintainability, controlled error handling
- Impact: Slower initial development than "full Blueprint", but better long-term quality

**ADR-002: C# for Script Tooling**
- Rationale: Better text processing ecosystem, easier CI integration
- Impact: Need to maintain cross-language data contracts and version compatibility

**Script Format:**
- YAML-based (`.narrrail.yaml` or `.narrrail.yml`)
- Schema version 1 (see `docs/SCRIPT_FORMAT.md`)
- Supports nodes (Dialogue/Choice/Jump/SetVariable/EmitEvent/End), edges with conditions, variables (Bool/Int/Float/String)

## Development Workflow

1. Make changes to `NarrRail/Source/...`
2. Run `HostProject/Build-HostProject.cmd` to compile
3. Open `HostProject/NarrRailHost.uproject` in UE5.7 editor to test
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

All tasks are tracked in `docs/TASK_PLAN.md` with unique IDs (e.g., `NR-RUN-001-01`).

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
- `docs/TECH_ARCHITECTURE.md`: Detailed architecture, module design, interface contracts
- `docs/TASK_PLAN.md`: WBS with completion tracking and DoD
- `docs/SCRIPT_FORMAT.md`: YAML script format specification v1
- This file: Development guidance for Claude Code

## Version Information

- Plugin version: 0.1.0 (beta/experimental)
- Target engine: UE5.7
- Script schema version: 1
- Platform: Windows (primary), cross-platform planned
