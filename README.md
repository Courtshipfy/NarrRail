# NarrRail

UE5.7 AVG plugin scaffold.

## Structure
- `NarrRail/NarrRail.uplugin`: plugin descriptor
- `NarrRail/Source/NarrRail`: runtime module
- `NarrRail/Source/NarrRailEditor`: editor module

## Docs
- `docs/TASK_PLAN.md`: WBS task checklist and supervision plan
- `docs/TECH_ARCHITECTURE.md`: technical architecture (C++/C# first, Blueprint as integration layer)

## Install (Project Plugin)
1. Copy the `NarrRail` folder into your UE project at `Plugins/NarrRail`.
2. Generate project files.
3. Build the project and enable the plugin in the editor.

## Next Steps
- Add data model for scripts and dialogue runtime.
- Implement script import/export.
- Build flow debugger UI.
