# NarrRail Constitution

## Core Principles

### I. Story Project Authoring First
NarrRail is an authoring product for Narrative Creators. Large features must strengthen the Story Project workflow from outline and structured script creation through review, preview, and export. A single `.nrstory` file is an asset inside a Story Project, not the top-level product unit.

### II. Neutral Story Format Ownership
The main repository owns the neutral `.nrstory`, `.nroutline`, and GlobalConfig contracts. Story Consumers, including the Unreal plugin repository, must declare compatibility with those contracts rather than adding private format semantics. Format changes require explicit compatibility and migration notes.

### III. Shell-Neutral Core, Adapter Boundaries
Shared parsing, serialization, validation, preview, review, import, and export behavior belongs in shell-neutral core modules. Browser, GitHub, local filesystem, future desktop shell, Git command, and future AI service behavior must sit behind adapters or product surfaces. Core code must not depend on Vue refs, DOM APIs, `fetch`, GitHub URLs, `localStorage`, or desktop-shell APIs.

### IV. Spec Before Large Execution
Large execution issues must move through a Spec Kit feature folder before implementation. The spec must define scope, non-goals, user workflows, data contracts, acceptance criteria, plan, tasks, and verification checklist. Small bug fixes, focused docs edits, and narrow test improvements may implement directly from the GitHub issue.

### V. Reviewable, Testable Changes
Features that affect story data, import/export behavior, project storage, review queues, preview semantics, or cross-file references must include regression or contract tests at the appropriate seam. Validation and review workflows must remain inspectable by users; AI or conversion output must not become authoritative project content without review.

## Product Constraints

- GitHub Issues are the source of truth for planning, assignment, and close state.
- `specs/` contains execution artifacts for large features; it does not replace GitHub Issues.
- The first-stage authoritative storage model is GitHub-backed Story Projects.
- NarrRail-owned backend database storage, hosted AI conversion service, and desktop packaging are deferred until explicit future issues reopen them.
- UI redesign should be Figma/design-system informed before large production UI rewrites.
- UE plugin work belongs to the Story Consumer split path and must not make Unreal the main repository's primary workflow again.

## Development Workflow

1. Use `wayfinder` for unclear product or architecture decisions.
2. Create or update GitHub issues when work becomes executable.
3. For large execution issues, use `$speckit-specify`, `$speckit-plan`, `$speckit-tasks`, and `$speckit-implement`, with the feature folder named `specs/<four-digit-issue-number>-<short-name>`.
4. Use `implement` directly only for small, fully specified changes.
5. Run relevant tests and builds before closing an issue.
6. Use `code-review` to compare implementation against both the issue and any spec folder.

## Governance

This constitution guides Spec Kit specifications, plans, tasks, and implementations. Changes require a commit that explains the product or engineering reason for the amendment and updates dependent docs when needed. If a GitHub issue or generated spec conflicts with this constitution, surface the conflict before implementation.

**Version**: 1.0.0 | **Ratified**: 2026-07-11 | **Last Amended**: 2026-07-11
