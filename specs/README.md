# NarrRail Specs

This directory holds Spec Kit execution specs for large NarrRail features.

Use `Docs/agents/spec-kit.md` for the workflow.

## Layout

```text
specs/
  0059-import-package-review/
    spec.md
    plan.md
    tasks.md
    checklist.md
```

Feature folders use the four-digit GitHub issue number plus a short kebab-case name.

## Status

Create a feature folder only when a large execution issue is ready for specification. Do not backfill closed issues unless a new implementation pass depends on them.

Official templates live under `.specify/templates/`. Do not add ad hoc templates under `specs/`.
