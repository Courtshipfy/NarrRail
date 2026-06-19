# NarrRail Outline Extension Migration Plan

## Goal

Move the external file naming from the current `.nrrail` outline suffix to a clearer `.nroutline` suffix, while keeping old `.nrrail` files readable.

Target suffixes:

- `.nrstory`: single story script
- `.nroutline`: story outline / rail graph
- `.nrconfig`: global project configuration

This migration should change the repository-facing file semantics first. Internal code names such as `railNodes`, `railMeta`, and `rail-yaml.js` can stay unchanged for now to keep the change scoped.

## Compatibility Rules

- New outline files should be created as `Stories/main_story.nroutline`.
- Existing `.nrrail` files must remain readable and editable.
- If both `Stories/main_story.nroutline` and an old `.nrrail` exist, prefer `.nroutline`.
- Saving an opened `.nrrail` may continue writing to that same path unless a later explicit migration UX is added.
- New exports should default to `.nroutline`.

## Implementation Checklist

### GitHub File Listing

- Update `NarrRailEditor/api/github/file-content.js`.
- Recognize `.nrstory`, `.nroutline`, and legacy `.nrrail`.
- Treat `.nroutline` and `.nrrail` as outline files.
- Continue parsing `meta.storyId` for `.nrstory`.
- Continue parsing `meta.railId` for `.nroutline` and `.nrrail`.
- Fall back to file stem only when the meta ID is missing.

### Script Library

- Update `NarrRailEditor/src/components/ScriptLibraryPage.vue`.
- `openOutlineRail()` should search for `.nroutline` first, then `.nrrail`.
- If neither exists in GitHub mode, create `Stories/main_story.nroutline`.
- If neither exists in local mock mode, create `Stories/main_story.nroutline`.
- Rename display helpers should strip `.nrstory`, `.nroutline`, and `.nrrail`.
- Search/filter/delete/rename flows should treat `.nroutline` and `.nrrail` as outline assets, not normal story scripts.

### Editor Open / Import / Export

- Update `NarrRailEditor/src/App.vue`.
- File input should accept `.nrstory`, `.nroutline`, and `.nrrail`.
- Import should route both `.nroutline` and `.nrrail` through the outline importer.
- GitHub save should treat both `.nroutline` and `.nrrail` as outline paths.
- New outline export should download as `.nroutline`.
- Existing opened legacy `.nrrail` should preserve its path on GitHub save.

### YAML Utilities

- Update `NarrRailEditor/src/utils/rail-yaml.js`.
- Default downloaded outline filename should end with `.nroutline`.
- Keep YAML schema fields unchanged for now:
  - `meta.railId`
  - `nodeType: Story | Branch | Note | End`

### Documentation

- Update `Docs/02_runtime/SCRIPT_FORMAT.md`.
- Replace primary `.nrrail` references with `.nroutline`.
- Add a compatibility note: `.nrrail` is a legacy outline suffix still supported by the editor.
- Update `Docs/06_planning/TASK_PLAN.md` references.

## Non-Goals For This Branch

- Do not rename internal variables/functions from `rail` to `outline`.
- Do not change YAML schema field names such as `railId`.
- Do not add a destructive migration that deletes old `.nrrail` files.
- Do not change UE plugin behavior yet.

## Acceptance Criteria

- A cloud repository with no outline file creates `Stories/main_story.nroutline` when clicking "打开剧情总纲".
- A cloud repository with old `Stories/main_story.nrrail` still opens correctly.
- A repository with both `.nroutline` and `.nrrail` opens `.nroutline`.
- Outline preview can resolve `.nrstory` references by internal `meta.storyId`, not by filename.
- Importing `.nroutline` works.
- Importing legacy `.nrrail` works.
- Exporting a new outline downloads `.nroutline`.
- `npm run build` passes.
