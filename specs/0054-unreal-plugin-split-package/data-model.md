# Data Model: NarrRail Unreal Plugin Split Package

## Split Package

Represents the prepared future repository.

Fields:

- Root path: `split-packages/NarrRail-Unreal-Plugin/`
- Package name: `NarrRail-Unreal-Plugin`
- Positioning: Story Consumer
- Contents: plugin, host, docs, README, manifest, ignore file

Validation rules:

- Must not require deleting files from the main repo.
- Must not claim ownership of `.nrstory` semantics.
- Must include links back to the main neutral format contract.

## Copied Source

Represents a clean-copied directory or file.

Fields:

- Source path
- Target path
- Purpose
- Exclusions

Validation rules:

- Source path must exist in the main repository.
- Target path must exist in the package after copy.
- Local/generated files such as `.DS_Store`, build output, and intermediate artifacts should be excluded.

## Split Manifest

Represents package provenance.

Fields:

- Source repository
- Package date
- Copy strategy
- Source-to-target mapping
- Exclusions
- Follow-up cleanup notes

Validation rules:

- Must list all top-level copied source paths.
- Must state that history-preserving extraction is out of scope.
- Must state that main-repo Unreal removal is out of scope.
