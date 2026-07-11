# Product Technology Shape Evaluation

Research issue: [Evaluate product technology shape without premature lock-in](https://github.com/Courtshipfy/NarrRail/issues/39)

## Question

How should NarrRail evaluate Web App, desktop client, and shared-core architectures without prematurely locking the technology stack before the product capability boundary is clear?

## Current Code Facts

- `NarrRailEditor` is a Vue 3 + Vite application.
- The existing editor already has pure JavaScript utilities for YAML import/export, validation, graph/story normalization, and preview running.
- GitHub-backed storage is already present through API routes under `NarrRailEditor/api/github/`.
- Local browser fallback currently uses `localStorage`, which is useful as a safety net but not enough for a GitHub-backed Story Project workbench.
- The current frontend is still mostly single-app shaped: UI, storage, GitHub integration, validation, and preview are close together rather than separated into a product core plus shell adapters.

## External Technology Facts

- Web-only can continue to support GitHub-backed authoring through GitHub APIs and browser storage, but browser local filesystem access is constrained by web platform permissions and browser support. See MDN's File System API documentation: https://developer.mozilla.org/en-US/docs/Web/API/File_System_API
- Tauri is a desktop shell that exposes native capabilities such as filesystem access through explicit plugins and permissions. See Tauri filesystem plugin documentation: https://v2.tauri.app/plugin/file-system/
- Electron provides a Chromium + Node.js desktop application model with main and renderer processes. See Electron process model documentation: https://www.electronjs.org/docs/latest/tutorial/process-model

## Evaluation Criteria

Use these criteria before choosing a shell:

- Story Project storage: Can the product read, write, validate, review, and commit the standard `Stories/` + `Config/` project layout?
- GitHub-backed workflow: Can the product preserve repo, branch, commit, and sync semantics without hiding source-of-truth behavior from creators?
- Local files and offline use: Can the product open a local project directory, work offline, and reconcile changes later?
- Core reuse: Can `.nrstory`, `.nroutline`, validation, preview, import package review, and export logic run outside any specific UI shell?
- UI reuse: Can the current Vue/Vite editor and planned Figma-driven redesign be reused with minimal shell-specific branching?
- Packaging and maintenance: How much build, signing, auto-update, and platform-specific work does the shell add?
- Security and permissions: Does the shell force broad filesystem/network access, or can permissions stay narrow and understandable?
- Future AI service fit: Can the shell support both user-provided API/service access and future NarrRail-hosted AI conversion without entangling AI with local editing?

## Recommended Architecture Direction

Do not choose Web-only, Tauri, or Electron as the product identity yet. Choose a shell-neutral architecture first:

```text
NarrRail Authoring UI
  uses
Story Project Application Services
  uses
NarrRail Shared Core
  - .nrstory parser/serializer
  - .nroutline parser/serializer
  - validation
  - preview runner
  - import package review model
  - export semantics
  uses
Shell Adapters
  - GitHub API adapter
  - browser storage adapter
  - local filesystem adapter
  - Git command / sync adapter
  - AI conversion service adapter
```

The next code-direction decision should be to extract and stabilize the Shared Core and application-service boundaries, not to migrate the whole app into a desktop shell.

## Candidate Shells

### Web-only

Best first-stage fit when the authoritative storage is GitHub and the product is still validating its Story Project workflow.

Use when:
- GitHub-backed projects remain the main storage mode.
- Figma-first redesign and Story Project IA are still evolving.
- Local filesystem/offline support is useful but not product-defining yet.

Limitations:
- Local directory workflows and offline editing are constrained.
- Browser storage is not enough for serious project persistence.
- Native packaging and system integration are deferred.

### Tauri

Strong candidate for the later standalone client if NarrRail wants local project directories, offline work, and a lighter native shell around the existing web UI.

Use when:
- The app needs local filesystem project access.
- The existing Vue/Vite UI should remain mostly reusable.
- The product wants a native desktop distribution without adopting a Node-heavy shell by default.

Risks:
- Requires Rust/Tauri build and permission model work.
- Native capabilities must be routed through explicit command/plugin boundaries.
- Git operations and updater/signing workflows need separate validation.

### Electron

Useful fallback if NarrRail needs broad Node.js ecosystem access, mature desktop integration patterns, or easier Git/file tooling integration at the cost of a heavier runtime.

Use when:
- The product needs deep Node-based local tooling.
- Git CLI integration and desktop automation matter more than bundle size.
- Team familiarity favors Electron strongly enough to offset maintenance/runtime cost.

Risks:
- Heavier runtime and packaging footprint.
- Security boundaries need careful main/renderer design.
- It can tempt the codebase to mix UI, filesystem, and process logic too freely.

## Recommended Stage Plan

1. Keep the current Vue/Vite web app as the active product surface while the Story Project IA and Figma prototypes mature.
2. Extract a shell-neutral NarrRail Shared Core for story parsing, serialization, validation, preview semantics, `.nroutline`, and import review.
3. Introduce storage/sync adapters behind interfaces:
   - GitHub API project adapter
   - Browser/local draft adapter
   - Future local filesystem project adapter
4. Build the redesigned Story Project workbench against those adapters.
5. Re-evaluate desktop shell only when local project directory, offline-first work, packaging, or OS integration becomes a real product requirement.
6. Prefer Tauri as the first desktop spike unless a concrete requirement demands Electron's Node-first ecosystem.

## Decision

NarrRail should avoid locking into Web-only, Tauri, or Electron during this planning stage. The stable decision is to make the product **shell-neutral with a shared authoring core**, continue using the current Vue/Vite web surface while design/product boundaries stabilize, and treat Tauri/Electron as later shell candidates evaluated by concrete local/offline/packaging needs.

## Follow-up For Code Boundaries

The next boundary investigation should identify:

- Which files move into a shared core package.
- Which APIs become application services.
- Which current browser/GitHub/localStorage behaviors become adapters.
- Which UI components must stop importing persistence or GitHub details directly.
