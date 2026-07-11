# Research: NarrRail Unreal Plugin Split Package

## Decision: Create a prepared package under `split-packages/`

**Rationale**: Issue #54 accepts a prepared split package. Creating it inside the current repository keeps the work reviewable without requiring immediate remote repository creation or credentials.

**Alternatives considered**:

- Create a sibling local repository outside the workspace: rejected because it is harder to review from the main repo and may require filesystem permissions outside the workspace.
- Remove Unreal files from the main repo immediately: rejected because the issue explicitly defers cleanup to a later ticket.

## Decision: Use clean copy rather than history-preserving extraction

**Rationale**: The ADR and issue both call for a clean copy. The original repository keeps history, while the new package can present a cleaner Story Consumer shape.

**Alternatives considered**:

- `git filter-repo` / subtree extraction: rejected as out of scope.
- Manual re-authoring of only selected files: rejected because it risks missing plugin or host dependencies.

## Decision: Keep UE docs together in the package

**Rationale**: The target repository should be useful on its own. Setup, Blueprint, debugger, UI integration, host project, architecture, and compatibility docs are all consumer-side docs.

**Alternatives considered**:

- Link back to all main-repo docs: rejected because the new repository should be self-describing.
- Copy only compatibility docs: rejected because issue #54 asks for Unreal-facing docs broadly.
