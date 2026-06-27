# NarrRailEditor

[中文](./README.md)

NarrRailEditor is a **Vue 3 + Canvas** visual story editor for NarrRail. It supports graph authoring, `.nrstory` import/export, validation, autosave, preview review, and script library workflows.

## Tech Stack

- **Vue 3** for the main UI, property panels, script library, and state management
- **Canvas Graph Renderer** for graph drawing, edges, dragging, panning, and zooming
- **YAML** serialization with the unified `.nrstory` extension
- **Vite** for development and production builds

## Quick Start

Online entry:

- https://narr-rail.courtship.top

Recommended flow:

1. Sign in and open Script Library
2. Create or open a `.nrstory`
3. Edit nodes and run validation
4. Use Preview Mode to review flow
5. Export `.nrstory` and commit it to the story repository
6. Sync the repository into UE with `Sync Stories`

## Local Development

Requirements:

- Node.js `>= 20`
- npm `>= 9`

```bash
cd NarrRailEditor
npm ci
npm run dev -- --host 127.0.0.1
```

Default URL: `http://127.0.0.1:5173/`

Cross-platform rule:

- Do not commit `node_modules`
- Do not copy `node_modules` between Windows, macOS, and Linux
- Run `npm ci` again after switching machines or platforms

Production build:

```bash
npm run build
```

## Current Features

### Graph Editor

- Node types: `Dialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `Condition`, `End`
- `Dialogue` nodes support single-line and multi-line dialogue editing
- Create, drag, connect, delete, and select nodes
- Context-menu node creation
- Auto layout
- Mouse wheel zoom and touchpad drag/pan/zoom support

### Data Editing

- Dynamic property forms by node type
- Variable management for script/global variables, types, and defaults
- `SetVariable` nodes can select existing variables
- `Condition` nodes can select existing variables
- Preset speaker management

### Condition Nodes

Condition is now the only supported condition-branching mechanism. Edge-level `condition` fields are deprecated and rejected by import/validation.

Current model:

- A Condition node can contain multiple branches
- Each branch maps to one output handle: `condition-0`, `condition-1`, `condition-2`, ...
- Branches are evaluated from top to bottom; the first matching branch wins
- If no branch matches, runtime uses `condition-fallback`
- Double-click a Condition node to open the dedicated editor modal
- The editor validates missing, duplicated, or out-of-range condition handles

### Import / Export

- `.nrstory` import
- `.nrstory` export
- Export never writes deprecated `edges[].condition`
- Import reports old edge conditions and asks users to use Condition nodes instead
- Global config files are supported with `meta.configType: GlobalConfig`

### Validation & Save

- Real-time validation + manual validation
- Entry node, ID uniqueness, edge references, orphan nodes, variable references, and Condition branch outputs
- Local autosave fallback

### Preview Mode

- Runtime-like execution order instead of canvas-position order
- Supports `Dialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `Condition`, and `End`
- Supports `ExhaustiveUntilComplete` choice traversal

## File Naming

Story scripts use the unified extension:

```text
*.nrstory
```

Global config also uses `.nrstory`. Recommended names:

```text
globalconfig.nrstory
global-config.nrstory
```

Global config files must declare:

```yaml
meta:
  schemaVersion: 1
  configType: GlobalConfig
variables: []
presetSpeakers: []
```

Old double-extension names such as `*.narrrail.yaml`, `*.narrrail.yml`, and `*.narrrail.nrstory` are no longer the default convention.

## UE Integration

NarrRailEditor authors and exports `.nrstory`; the UE plugin syncs the local story repository into assets.

UE-side flow:

1. Set `Story Repository Path` in `Project Settings > Plugins > NarrRail`
2. Click `Sync Stories` in the toolbar
3. The plugin runs `git pull --ff-only` by default when the folder is a Git repository
4. The plugin mirrors the repository into `/Game/NarrRailStories/<repository-folder-name>`
5. Story files become `UNarrRailStoryAsset`
6. Global config files become `UNarrRailGlobalConfigAsset`
7. Assets for deleted repository files are removed after confirmation
