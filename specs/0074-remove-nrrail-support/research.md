# Research: Remove `.nrrail` Support

## Decision: Treat the suffix removal as a file-contract change, not a YAML migration

**Rationale**: Both old and new outline files use the same outline YAML fields,
including `meta.railId`. The requested removal concerns how paths are accepted
and produced, so changing YAML fields would create unrelated compatibility work.

**Alternatives considered**:

- Keep a read-only `.nrrail` path: rejected because the user explicitly chose
  direct removal.
- Automatically rename `.nrrail` files: rejected because it mutates a
  GitHub-backed Story Project and was explicitly out of scope.

## Decision: Reject retired files by making them unknown assets

**Rationale**: Project core classification is the shared boundary used by review
and future project surfaces. Returning `unknown` ensures a retired path cannot
silently participate as an outline.

**Alternatives considered**:

- Classify it as an outline with a warning: rejected because that retains active
  support.
- Throw during every project scan: rejected because unrelated repository files
  are already valid unknown assets.

## Decision: Keep historical records, update active contract surfaces

**Rationale**: Closed specs and research documents record decisions made at the
time. Active README, neutral contract, runtime reference, ADR, migration
guidance, source, and tests must describe the present `.nroutline`-only rule.

**Alternatives considered**:

- Rewrite all historical artifacts: rejected because it obscures project history
  without changing runtime behavior.
