# NarrRail

[中文](./README.md)

NarrRail is an authoring and script-assistance product for interactive narrative projects. The main repository owns the Story Project workflow, `.nrstory` / GlobalConfig / `.nroutline` neutral format contracts, import review, preview validation, and conversion workflows.

The Unreal Engine plugin and sample host now live in the separate Story Consumer repository:

<https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>

## Product Focus

NarrRail's first-stage user is the Narrative Creator: a writer, narrative designer, or creator who needs to organize branching story structure.

Core workflow:

```text
idea / outline
  -> Story Project organization
  -> structured script editing
  -> branches, variables, and conditions
  -> Project Review Queue
  -> Project Preview
  -> export .nrstory / .nroutline for Story Consumers
```

## Story Project

Story Project is the project-level authoring workspace. The first-stage storage model is a GitHub-backed repository or repository-like folder.

Recommended structure:

```text
Stories/
  main_story.nroutline
  chapter_01.nrstory
Config/
  global-config.nrstory
```

- `.nrstory`: story script content with nodes, edges, dialogue, choices, conditions, variable operations, events, and endings.
- `.nroutline`: project-level story outline that composes multiple `.nrstory` files into chapters, routes, or a larger structure.
- `Config/global-config.nrstory`: project-level variables and preset speakers.

Legacy `.nrrail` files are compatibility-only outline files. New outlines should use `.nroutline`.

## Current Authoring App

`NarrRailEditor/` is the current Vue 3 + Vite authoring app.

Current capabilities:

- Graph editing for `Dialogue`, `MultiDialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `Condition`, and `End`
- Multi-branch Condition nodes with `condition-N` outputs and `condition-fallback`
- Global variables and preset speakers
- `.nrstory` import/export
- `.nroutline` / legacy `.nrrail` migration path
- Real-time and manual validation
- Local autosave fallback
- Runtime-semantics preview
- GitHub-backed script library and config sync

Local development:

```bash
cd NarrRailEditor
npm ci
npm run dev -- --host 127.0.0.1
```

Validation:

```bash
cd NarrRailEditor
npm test
```

## Script Conversion

NarrRail Story Converter is a project-level Codex skill for converting writer-provided Word, Excel, CSV, TXT, screenplay drafts, scene outlines, or loose prose into structured NarrRail story files.

Short-term goals:

- Generate `.nrstory` from source manuscripts
- Output `conversion-notes.md`
- Handle uncertainty conservatively
- Validate structure against NarrRail rules

Medium-term direction:

- Emit Story Project Import Packages
- Support `Config/conversion-profile.md`
- Route generated content through Import Package Review before merging into the authoritative Story Project

## Story Consumers

The main repository does not carry active Story Consumer implementation code. Story Consumers read NarrRail story files and declare compatibility with the main repository's neutral format contracts.

Unreal consumer:

- Repository: <https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>
- Main-repo pointer: `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md`

Format changes requested by a consumer should start in this main repository, because this repository owns `.nrstory`, GlobalConfig, and `.nroutline` semantics.

## Important Docs

- `CONTEXT.md`: domain language
- `Docs/spec/NRSTORY_FORMAT.md`: neutral `.nrstory` / GlobalConfig / `.nroutline` format contract
- `Docs/02_runtime/SCRIPT_FORMAT.md`: runtime-facing format reference and historical entry point
- `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md`: Unreal Story Consumer repository pointer
- `Docs/adr/0001-split-unreal-consumer-repository.md`: repository split decision
- `Docs/agents/spec-kit.md`: Spec Kit workflow for large execution issues
- `NarrRailEditor/README.md`: current authoring app guide
- `.codex/skills/narrrail-story-converter/`: manuscript conversion skill
