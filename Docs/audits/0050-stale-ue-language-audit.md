# Audit: stale UE-first language after Unreal split

Issue: [#50](https://github.com/Courtshipfy/NarrRail/issues/50)

Date: 2026-07-11

## Scope

Audited the active main-repo surfaces for stale Unreal-first framing after the
Unreal consumer split:

- Top-level README files
- `Docs/`
- `NarrRailEditor` README files
- `NarrRailEditor/src/`
- Project Codex skills

Historical `Docs/research/`, `Docs/adr/`, and completed `specs/` records were
not rewritten unless they created live product guidance. Those files are allowed
to mention the old coupling as decision history.

## Changed

- Reframed the NarrRailEditor overview page around Story Project authoring,
  project preview, import review, and Story Consumer export.
- Replaced editor README instructions that told users to sync into UE with
  neutral Story Consumer handoff language.
- Replaced embedded UE-side Sync Stories, Blueprint, PIE, asset mapping, and host
  workflow instructions with a link to `NarrRail-Unreal-Plugin`.
- Updated the Overview page content spec so it says the Unreal plugin has already
  moved to the external repository.
- Updated the outline migration planning note so it no longer points to deleted
  planning docs or talks about changing local UE plugin behavior.

## Intentionally Remaining

- `README.md`, `README.en.md`, `Docs/spec/NRSTORY_FORMAT.md`,
  `Docs/02_runtime/SCRIPT_FORMAT.md`, and
  `Docs/story-consumers/NARRRAIL_UNREAL_PLUGIN.md` still mention Unreal as a
  supported external Story Consumer.
- Platform-specific terms such as Blueprint, PIE, HUD, Sync Stories, and Content
  path remain only where the text says those details belong in the external
  Unreal consumer repository.
- `Docs/research/` and `Docs/adr/` keep historical references to the former
  main-repo Unreal coupling.

## Result

The active main-repo product surfaces now present NarrRail as an authoring
product and format owner. Unreal references that remain are consumer-facing and
link out to:

<https://github.com/Courtshipfy/NarrRail-Unreal-Plugin>
