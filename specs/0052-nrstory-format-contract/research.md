# Research: Neutral .nrstory Format Contract

## Decision: Add a new neutral contract under `Docs/spec/`

**Rationale**: `Docs/02_runtime/SCRIPT_FORMAT.md` is a historical runtime-facing path and already has downstream references. A new `Docs/spec/NRSTORY_FORMAT.md` gives the main repository a clear neutral source of truth without breaking existing links.

**Alternatives considered**:

- Replace `Docs/02_runtime/SCRIPT_FORMAT.md` entirely: rejected because existing docs and conversion references still point to it.
- Keep only the existing runtime doc: rejected because the issue asks for a neutral contract and clear Story Consumer boundary.

## Decision: Keep UE-specific sync details outside the neutral contract

**Rationale**: UE sync, assets, Blueprint, PIE, HUD, and Content paths are Story Consumer implementation behavior. They belong in the future `NarrRail-Unreal-Plugin` repository or compatibility matrix, not in the main format contract.

**Alternatives considered**:

- Include a UE mapping appendix in the neutral contract: rejected because it would keep Unreal as the implied primary consumer.
- Delete all UE references from the repo now: rejected because #52 is not the UE code/doc migration ticket.

## Decision: Remove obsolete non-Codex integration records now

**Rationale**: The user explicitly requested a Codex-only setup. The repository already has missing managed skill files for the obsolete integration, causing Spec Kit status to fail while the stale integration is still registered.

**Alternatives considered**:

- Reinstall the obsolete integration skills: rejected because it contradicts the user request.
- Only remove README references: rejected because Spec Kit integration metadata would remain inconsistent.
