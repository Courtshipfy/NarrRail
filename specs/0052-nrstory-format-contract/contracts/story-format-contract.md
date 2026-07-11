# Contract: Neutral Story Format Ownership

## Main Repository Responsibilities

- Own the neutral `.nrstory`, GlobalConfig, and `.nroutline` field semantics.
- Define current schema versions and compatibility expectations.
- Document validation expectations used by authoring, conversion, review, preview, and export workflows.
- Record schema migrations or compatibility changes through explicit future issues.

## Story Consumer Responsibilities

Each Story Consumer must declare:

- Supported Story Script `schemaVersion` values.
- Supported GlobalConfig `schemaVersion` values.
- Whether `.nroutline` is supported.
- Whether unknown fields are preserved, ignored, or rejected.
- Unsupported node types, action types, event conventions, choice timers, or outline features.

Each Story Consumer may:

- Convert neutral story files into runtime assets or memory structures.
- Add platform-specific setup, sync, debugging, and runtime APIs in its own documentation.

Each Story Consumer must not:

- Redefine neutral field semantics.
- Require platform-specific fields in neutral `.nrstory`, GlobalConfig, or `.nroutline` files.
- Treat higher unsupported schema versions as if they were fully compatible.

## Non-Goals

- No schema migration in this issue.
- No Unreal code movement in this issue.
- No new `NarrRail-Unreal-Plugin` repository creation in this issue.
