# NarrRail

UE5.7 AVG plugin scaffold.

## Structure
- `NarrRail/NarrRail.uplugin`: plugin descriptor
- `NarrRail/Source/NarrRail`: runtime module
- `NarrRail/Source/NarrRailEditor`: editor module
- `HostProject/NarrRailHost.uproject`: host sample project for plugin development

## Docs
- `docs/TASK_PLAN.md`: WBS task checklist and supervision plan
- `docs/TECH_ARCHITECTURE.md`: technical architecture (C++/C# first, Blueprint as integration layer)
- `docs/HOST_PROJECT.md`: host sample project setup and daily workflow
- `docs/SCRIPT_FORMAT.md`: script JSON format spec for import/export

## Install (Project Plugin)
1. Copy the `NarrRail` folder into your UE project at `Plugins/NarrRail`.
2. Generate project files.
3. Build the project and enable the plugin in the editor.

## Develop With Host Project
1. Open `HostProject/NarrRailHost.uproject` in UE5.7.
2. Double-click `HostProject/Build-HostProject.cmd` in Explorer.
3. Iterate on `NarrRail/Source/...` and rebuild.

## Next Steps
- Add data model for scripts and dialogue runtime.
- Implement script import/export.
- Build flow debugger UI.
