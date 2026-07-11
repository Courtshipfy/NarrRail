# Authoring Product Code Boundaries

Research issue: [Identify code boundaries needed for the new authoring-product direction](https://github.com/Courtshipfy/NarrRail/issues/37)

## Question

Which code and module boundaries across NarrRailEditor, `.nrstory` validation/import/export logic, Script Conversion rules, and project documentation should be cleaned up first to support the main repo's shift into an independent authoring product?

## Summary Decision

NarrRail should clean up code boundaries in this order:

1. Create a shell-neutral NarrRail Shared Core for story formats, validation, preview semantics, outline semantics, and import-review models.
2. Introduce Story Project application modules that operate on `Stories/`, `Config/`, `.nrstory`, `.nroutline`, and review items.
3. Move GitHub, browser storage, future filesystem, and future Git behavior behind storage/sync adapter seams.
4. Keep Vue components focused on interaction and presentation, especially as Figma-first UI work begins.
5. Reposition Script Conversion as an import workflow that depends on Shared Core validation instead of depending on editor utility files.
6. Split or retire UE-first documentation in the main repo as part of the consumer-repo split.

## Current Code Facts

- `NarrRailEditor` is a Vue 3 + Vite app.
- `NarrRailEditor/src/App.vue` is about 3,800 lines and currently mixes product state, view routing, GitHub fetches, localStorage fallback, import/export, validation scheduling, auto-layout, history, theme, and global config sync.
- `NarrRailEditor/src/utils/` already contains good shared-core candidates:
  - `yaml-importer.js`
  - `yaml-exporter.js`
  - `validation.js`
  - `story-preview-runner.js`
  - `rail-yaml.js`
  - `rail-validation.js`
  - `rail-preview-runner.js`
  - `global-config-yaml.js`
  - `dialogue-node.js`
- GitHub integration currently lives in serverless-style endpoints under `NarrRailEditor/api/github/`.
- Browser draft persistence currently uses `localStorage` through `NarrRailEditor/src/utils/storage.js` and additional direct `localStorage` calls in `App.vue`.
- The Script Conversion skill currently says to validate with `NarrRailEditor/src/utils/yaml-importer.js` and `NarrRailEditor/src/utils/validation.js`, which means conversion depends on editor internals rather than a format/core package.
- Main README and several docs still position NarrRail as a UE-centered toolchain, while the new map decision says the main repo should be an authoring product and story-format owner.

## Proposed Modules And Seams

### NarrRail Shared Core

This should be the deepest module. Its interface should be small and boring; callers should not know whether they are in Vue, a skill, a CLI, Tauri, Electron, or GitHub mode.

Candidate interface:

```text
parseStory(yamlText) -> StoryDocument
serializeStory(storyDocument) -> yamlText
validateStoryDocument(storyDocument, context) -> ReviewItems
parseOutline(yamlText) -> OutlineDocument
serializeOutline(outlineDocument) -> yamlText
validateOutlineDocument(outlineDocument, context) -> ReviewItems
runStoryPreview(storyDocument, config, options) -> PreviewSession
runProjectPreview(projectSnapshot, options) -> PreviewSession
```

Initial implementation should reuse the existing editor utils rather than rewrite them.

Belongs here:

- `.nrstory` parser/serializer
- `.nroutline` parser/serializer
- legacy `.nrrail` reader compatibility
- `GlobalConfig` parser/serializer
- story validation
- outline validation
- preview runner semantics
- review item data model
- import package validation primitives

Does not belong here:

- Vue refs/reactivity
- DOM download behavior
- `fetch`
- GitHub URLs
- localStorage
- file picker UI
- Figma/design-system concerns

### Story Project Application Modules

These modules express product workflows over a project snapshot.

Candidate interfaces:

```text
loadProject(adapter, projectRef) -> ProjectSnapshot
listProjectAssets(projectSnapshot) -> ProjectAssetIndex
reviewProject(projectSnapshot) -> ProjectReviewQueue
applyImportPackage(projectSnapshot, importPackage) -> ProjectChangeSet
prepareExport(projectSnapshot, target) -> ExportBundle
```

Belongs here:

- `Stories/` + `Config/` project layout rules
- `.nroutline` orchestration rules
- cross-file references
- Project Review Queue aggregation
- Import Package Review
- Project Preview orchestration
- export package construction

Does not belong here:

- Button click handlers
- direct GitHub REST calls
- Tauri/Electron filesystem commands
- AI provider calls

### Storage And Sync Adapters

Adapters should satisfy project storage interfaces. They can be shallow, because the depth belongs in Shared Core and Story Project modules.

Likely adapters:

- GitHub repository adapter: current first-stage authoritative storage.
- Browser draft adapter: local autosave and emergency recovery only.
- Local filesystem adapter: future desktop/offline candidate.
- Git command adapter: future desktop sync candidate.
- AI conversion adapter: future hosted or user-provided conversion service integration.

The important seam is not "GitHub API endpoint per UI action"; it is "load/save/commit project assets through a project storage interface."

### Vue Authoring UI

Vue should become the interaction shell over application modules.

Belongs here:

- Figma-driven layout
- Project Dashboard
- Structured Script View
- Graph View
- Review Queue UI
- Preview UI
- Config/Profile UI
- transient selection, focus, modal, and keyboard state

Should move out:

- YAML conversion rules
- cross-file project validation
- GitHub fetch construction
- direct localStorage project persistence
- preview semantics
- import package interpretation

## First Cleanup Priorities

### 1. Shared Core Package Or Folder

Create a clear home such as:

```text
NarrRailEditor/src/core/
```

or, if moving toward a multi-package repo:

```text
packages/narrrail-core/
```

Recommendation for the first refactor: start inside `NarrRailEditor/src/core/` to reduce packaging churn, then lift to `packages/narrrail-core/` after tests and call sites stabilize.

Move or wrap:

- `yaml-importer.js`
- `yaml-exporter.js`
- `validation.js`
- `story-preview-runner.js`
- `rail-yaml.js`
- `rail-validation.js`
- `rail-preview-runner.js`
- `global-config-yaml.js`

### 2. Project Asset And Review Model

Add explicit product-level concepts before building the Project Dashboard:

- `StoryProject`
- `ProjectAsset`
- `ProjectSnapshot`
- `ProjectReviewItem`
- `ImportPackage`
- `ConversionProfile`

This enables Dashboard, Review Queue, Import Package Review, and Project Preview to share one model.

### 3. Storage Adapter Seam

Introduce a project storage interface before adding desktop/file APIs:

```text
listAssets()
readAsset(path)
writeAsset(path, content, options)
deleteAsset(path, options)
commit?(message)
getStatus?()
```

The GitHub implementation can wrap the existing `/api/github/*` routes. Browser draft storage should not masquerade as authoritative project storage.

### 4. App.vue Decomposition

Do not split by arbitrary line count. Split by seam:

- `useAuthSession`
- `useThemePreference`
- `useEditorHistory`
- `useGraphSelection`
- `useStoryAsset`
- `useOutlineAsset`
- `useProjectStorage`
- `useProjectReview`

This should follow the shared core and adapter work, otherwise the composition functions will just preserve the same coupling in smaller files.

### 5. Conversion Workflow Boundary

Update the Script Conversion skill and future product importer so conversion produces a Story Project Import Package, then validates it through Shared Core.

The converter should not depend on `NarrRailEditor/src/utils/*` as editor internals. It should depend on the shared format/core interface.

### 6. Documentation Split

Main repo docs should be repositioned around:

- authoring product
- story format ownership
- conversion workflow
- Story Project layout
- consumer compatibility

UE runtime docs should either move to the future `NarrRail-Unreal-Plugin` repo or be clearly marked as legacy/consumer-side until the split happens.

## UE-Facing Coupling To Remove Or Isolate

- Main README still presents the product as Editor + Converter + UE plugin + UE host. This should change after or during the repository split.
- `Docs/01_architecture/TECH_ARCHITECTURE.md` is still C++/UE-centered and no longer describes the intended main repo architecture.
- `Docs/05_narrrail_editor/INTRO_PAGE_CONTENT_STYLE_SPEC.md` still centers the intro page on UE runtime capability.
- `Docs/06_planning/TASK_PLAN.md` contains many UE delivery milestones that should not remain the main authoring-product backlog.
- Script format docs should keep consumer compatibility notes, but avoid treating UE import as the main reason the format exists.

## Test Gaps Blocking Safe Follow-Up Work

Current test command:

```bash
cd NarrRailEditor
npm test
```

Current result:

```text
event model tests passed
```

Current coverage is too narrow for the coming refactor. Add tests before or during extraction for:

- `.nrstory` parse -> validate -> serialize round trips.
- legacy and current Choice/Condition cases.
- `GlobalConfig` parse/serialize/validation.
- `.nroutline` and legacy `.nrrail` parse/validate/serialize.
- story preview runner behavior for Dialogue, MultiDialogue, Choice, SetVariable, EmitEvent, Condition, Jump, and End.
- Project Review Queue aggregation from multiple files.
- GitHub adapter contract with a fake transport.
- Story Project Import Package validation.
- Structured Script View row-model conversions once that prototype starts.

## Recommended Follow-Up Tickets

1. Extract NarrRail shared core from editor utilities.
2. Define Story Project domain model and Project Review Queue model.
3. Introduce project storage adapter interface and GitHub adapter.
4. Reposition main documentation around authoring product and move UE-facing docs toward the consumer-repo split.
5. Expand core regression tests before large UI/UX implementation.

## Decision

The first code cleanup should not be a broad UI rewrite and should not be a desktop-shell migration. The next implementation work should create deep modules at the story-format, project-workflow, and storage-adapter seams, then let the Figma-first UI redesign call into those modules.
