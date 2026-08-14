# NarrRail Outline Suffix Cutover

## Current Rule

`.nroutline` is the only supported Story Outline suffix in NarrRail. The
authoring product recognizes, creates, imports, exports, saves, and renames
Story Outlines only with this suffix.

The stable outline YAML fields remain unchanged, including `meta.railId` and
the `Story`, `Branch`, `Note`, and `End` node types. Internal code may still use
the established `rail` terminology.

## Retired Files

`.nrrail` is retired and unsupported. NarrRail does not classify it as a Story
Outline, show it in the Script Library, open it as an outline, or write it.

To use an existing retired file, rename it outside NarrRail from `*.nrrail` to
`*.nroutline` before opening the Story Project. This is intentionally an
external repository/file operation: NarrRail does not perform automatic renames
or retain a compatibility reader.

## Expected Project Layout

```text
Stories/
  main_story.nroutline
  chapter_01.nrstory
Config/
  global-config.nrstory
```

## Acceptance Criteria

- A project without an outline creates `Stories/main_story.nroutline`.
- Project scans classify only `.nroutline` files as Story Outlines.
- The editor file picker accepts `.nrstory` and `.nroutline`.
- Outline downloads end with `.nroutline`.
- `npm test` and `npm run build` pass in `NarrRailEditor`.
