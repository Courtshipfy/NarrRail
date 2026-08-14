# Story Outline File-Suffix Contract

## Supported suffix

NarrRail recognizes a Story Outline only when its path ends in `.nroutline`
(case-insensitive).

## Retired suffix

Paths ending in `.nrrail` are not Story Outline assets. NarrRail does not list,
open, import, export, rename, or save them as Story Outlines.

## Unchanged document structure

The Story Outline YAML document remains unchanged. It continues to use
`meta.railId` as its stable outline identifier.

## Migration responsibility

Migrating a retired file requires an external repository/file rename from
`*.nrrail` to `*.nroutline` before it is opened in NarrRail. NarrRail does not
perform that rename automatically.
