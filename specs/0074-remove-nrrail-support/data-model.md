# Data Model: Outline File Suffix

## Story Outline Asset

| Field | Value |
| --- | --- |
| Recognized path suffix | `.nroutline` |
| Project asset kind | `outline` |
| YAML identity field | `meta.railId` |
| Storage location | Usually `Stories/` in a Story Project |

## Retired Outline File

| Field | Value |
| --- | --- |
| Retired path suffix | `.nrrail` |
| Project asset kind | `unknown` |
| Import/export eligibility | Not eligible |
| Automatic migration | None |

## Invariants

- Path suffix controls whether a project file is recognized as an outline.
- The suffix change does not alter `meta.railId`, outline nodes, outline edges,
  or preview semantics.
- An unknown asset is retained only as an unrecognized repository file; it is
  never routed to the outline editor or review validator.
