# Data Model: Neutral .nrstory Format Contract

## Story Script

Represents a single interactive story file.

Required identity and structure:

- `meta.schemaVersion`
- `meta.storyId`
- `meta.entryNodeId`
- `nodes`
- `edges`

Validation rules:

- `entryNodeId` must exist in `nodes[].nodeId`.
- `nodeId` values must be non-empty and unique.
- Edges must reference existing source and target nodes.
- Story Script variables are reserved and must be empty or omitted.

## GlobalConfig

Represents project-level variables and preset speakers.

Required identity:

- `meta.schemaVersion`
- `meta.configType: GlobalConfig`

Validation rules:

- Variable names must be non-empty and unique.
- Variable type must be one of `Bool`, `Int`, `Float`, or `String`.
- Scope is currently `Global`.

## Story Outline

Represents Story Project-level composition across multiple Story Scripts.

Required identity and structure:

- `meta.schemaVersion`
- `meta.railId`
- `meta.entryNodeId`
- `nodes`
- `edges`

Validation rules:

- `entryNodeId` must exist in `nodes[].nodeId`.
- `Story` nodes must reference resolvable `meta.storyId` values when validated inside a Story Project.
- `Branch` source handles must match branch indexes or fallback.

## Story Consumer

Represents a downstream runtime, tool, or integration that reads NarrRail story files.

Required declarations:

- Supported Story Script schema versions.
- Supported GlobalConfig schema versions.
- `.nroutline` support status.
- Unknown-field retention behavior.
- Unsupported node/action/event/timer capabilities.

Compatibility rule:

- A Consumer may map neutral story data into platform-specific objects, but those objects do not define new neutral format semantics.
