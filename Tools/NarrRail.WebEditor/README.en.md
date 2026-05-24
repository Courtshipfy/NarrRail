# NarrRail WebEditor

[中文](./README.md)

NarrRail WebEditor is a **Vue 3 + Svelte Flow** visual story editor for narrative production.
It supports graph editing, `.nrstory` import/export, validation, autosave, and runtime-like preview.

## Tech Stack

- **Vue 3** for app-level UI
- **Svelte Flow (@xyflow/svelte)** for graph editing interactions
- **YAML** serialization with unified `.nrstory` extension
- **Vite** for dev/build

## Quick Start

### Online Entry (Default)

- https://narr-rail.courtship.top

### Recommended Usage Flow

1. Sign in and enter Script Library
2. Create/open a `.nrstory`
3. Edit nodes and run validation
4. Use Preview Mode to review flow
5. Export `.nrstory` and bring it back to UE

### Local Development (for maintainers)

Requirements:

- Node.js `>= 20` (LTS recommended)
- npm `>= 9`

Cross-platform rule:

- Do not commit `node_modules`, and do not copy it between Windows and macOS
- Reinstall dependencies per machine after switching platforms (`npm ci` or `npm install`)

macOS / Linux:

```bash
cd Tools/NarrRail.WebEditor
npm ci
npm run dev -- --host 127.0.0.1
```

Windows (PowerShell / CMD):

```bash
cd Tools/NarrRail.WebEditor
npm ci
npm run dev -- --host 127.0.0.1
```

Default URL: `http://127.0.0.1:5173/`

Common issues:

- `Cannot find module @rollup/rollup-<platform>`:
  remove `node_modules` and run `npm ci` again
- `node_modules/.bin/vite: Permission denied` (macOS/Linux):
  run `chmod +x node_modules/.bin/vite` and retry

Performance baseline (local dev):

1. Start the local dev server and open the editor page
2. Run in browser console:

```js
await window.__narrrailBench.runGraphSyncBenchmark()
```

Optional parameters:

```js
await window.__narrrailBench.runGraphSyncBenchmark({
  iterations: 300,
  moveEvery: 3,
  shiftPx: 1.5
})
```

The output includes `averageMs / p95Ms / p99Ms` for before/after optimization comparisons.

### Build

```bash
npm run build
```

## Implemented Features

### 1) Graph Editor Core

- Node types: `Dialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `End`
- Create/connect/move/delete/select nodes
- Context menu node creation
- Auto layout (including Choice branch fan-out optimization)
- Edge style toggle (straight / bezier)

### 2) Editing Panels

- Dynamic node property forms
- Edge properties: `priority`, `condition.logic`, `condition.terms`
- Variables and preset speakers management

### 3) Import / Export

- `.nrstory` import
- `.nrstory` export
- Auto layout after import
- Basic bidirectional mapping (graph ↔ script)

### 4) Validation & Save

- Real-time validation + manual validation
- Entry node / ID uniqueness / edge references / orphan node checks
- Local autosave fallback

### 5) Preview Mode

- Switch from graph mode to runtime-like preview mode
- Execution order follows runtime semantics (not canvas position)
- Supports: `Dialogue`, `MultiDialogue`, `Choice`, `Jump`, `SetVariable`, `EmitEvent`, `End`
- Supports `ExhaustiveUntilComplete`
- End-node duplicate-trigger prevention
- Timeline tags: `[Choice] [Variable] [Event] [Jump] [End]` with type-aligned colors

## File Naming Convention

### Story Scripts

- Unified extension: `.nrstory`

### Global Config

- Recommended names:
  - `globalconfig.nrstory`
  - `global-config.nrstory`

> Legacy double-suffix names (e.g. `*.narrrail.yaml` / `*.narrrail.yml` / `*.narrrail.nrstory`) are no longer default conventions.

## Related Docs

- Preview mode notes: `README_READING_MODE.md`
- Intro page spec: `../../Docs/05_webeditor/INTRO_PAGE_CONTENT_STYLE_SPEC.md`
- Deployment plan: `../../Docs/05_webeditor/WEBEDITOR_DEPLOYMENT_PLAN.md`

## License

Part of the NarrRail repository and follows repository-level licensing.
