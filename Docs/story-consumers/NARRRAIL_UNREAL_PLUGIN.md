# NarrRail Unreal Plugin

`NarrRail-Unreal-Plugin` is the Unreal Engine Story Consumer for NarrRail story projects.

Repository:

<https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>

The Unreal repository owns:

- UE plugin source
- UE sample/host project
- Blueprint integration docs
- Debugger, HUD, PIE, and Sync Stories docs
- Unreal-side compatibility matrix and setup instructions

The main NarrRail repository owns:

- `.nrstory` Story Script contract
- GlobalConfig contract
- `.nroutline` Story Outline contract
- authoring-side validation, preview, review, import, and export semantics

Unreal-side feature requests may reveal new format needs, but format changes must start in this main repository. A Story Consumer should declare support for main-repo format versions rather than adding private `.nrstory`, GlobalConfig, or `.nroutline` semantics.

Current neutral format contract:

- `Docs/spec/NRSTORY_FORMAT.md`
- `Docs/02_runtime/SCRIPT_FORMAT.md`
