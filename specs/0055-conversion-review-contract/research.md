# Research: Conversion Review Contract

## Decision: Make Markdown required when review items exist, JSON optional

**Rationale**: Issue #55 requires human-readable notes and optional machine-readable review metadata. Markdown is accessible without tooling; JSON exists for Review Queue ingestion.

**Alternatives considered**:

- JSON-only: rejected because reviewers need readable notes without tooling.
- Markdown-only: rejected because Project Review Queue would need brittle Markdown parsing.

## Decision: Use the existing Project Review Queue item vocabulary

**Rationale**: `NarrRailEditor/src/core/story-project.js` already has `severity`, `category`, `assetPath`, `target`, and `source`. Extending this seam keeps import-package review shell-neutral and avoids a parallel model.

**Alternatives considered**:

- New converter-only item model: rejected because future UI would need mapping glue.
- Store review metadata inside `.nrstory`: rejected by the neutral format constitution.

## Decision: Represent generated targets separately from source locations

**Rationale**: A review item can point to both source manuscript text and generated NarrRail output. Keeping both allows reviewers to trace an issue back to the manuscript while opening the generated node or asset when available.

**Alternatives considered**:

- Single `location` field: rejected because it conflates source and generated locations.

## Decision: Parse `conversion-review.json` from Import Package files

**Rationale**: The next Import Package Review workflow can include `conversion-review.json` as a file beside generated assets. Loading it in the core queue model proves the contract works before UI is built.

**Alternatives considered**:

- Only accept review items passed as in-memory `reviewItems`: rejected because it does not validate the file contract.
