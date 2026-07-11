# NarrRail

NarrRail is a script creation and authoring-assistance product for interactive narrative projects. Its primary language centers on writers, narrative designers, story structure, and exportable story formats.

## Language

**Narrative Creator**:
The primary first-stage user of NarrRail: a writer or narrative designer creating, organizing, reviewing, and exporting interactive scripts.
_Avoid_: Engine integrator, programmer, runtime user

**Story Consumer**:
A downstream tool or runtime that reads exported NarrRail story files and implements them in an engine or delivery environment.
_Avoid_: Primary user, authoring client

**Authoring Flow**:
The first-stage product workflow from idea or outline, through story structuring, branch and variable design, preview review, and export to `.nrstory`.
_Avoid_: Engine integration flow, runtime debugging flow

**Story Project**:
The top-level authoring workspace for a narrative work, containing global configuration, multiple story scripts, an outline, and the import, validation, preview, correction, and export workflow around them.
_Avoid_: Single file, isolated script

**GitHub-backed Story Project**:
A Story Project whose authoritative storage is a GitHub repository or repository directory containing the project's story files and configuration. Commit history, branches, and repository permissions provide the first-stage persistence and collaboration substrate.
_Avoid_: NarrRail-owned project database, optional GitHub export target

**Project Asset**:
A file-like item inside a Story Project, such as a `.nrstory`, `.nroutline`, GlobalConfig file, conversion profile, or conversion note. Project Assets are the units that project review, preview, import, and export flows inspect.
_Avoid_: UI card, engine asset, arbitrary repository file

**Project Snapshot**:
A read model of a Story Project at a point in time: project metadata, recognized Project Assets, import packages, conversion profiles, and indexes needed by review or preview flows.
_Avoid_: Persistent database record, live Git branch state, Vue component state

**Project Review Queue**:
A project-level review surface that gathers validation errors, warnings, and human-review items across configuration, story scripts, outlines, imports, and cross-file references.
_Avoid_: Per-file error popup, runtime debugger

**Project Preview**:
A preview flow that starts from the Story Project outline and runs across multiple story scripts with shared global configuration and variable state.
_Avoid_: Single-file preview only, engine runtime debug session

**Script Editor View**:
One of multiple editing and presentation views over the same `.nrstory` data. Graph-oriented and text-oriented views are peer views of one story structure, not separate products or separate story formats.
_Avoid_: Separate file format, duplicated editor data, graph-only authoring model

**Structured Script View**:
A text-like Script Editor View where each row is an explicit NarrRail story element, such as dialogue, narration, event, variable operation, condition, branch choice, or jump. It gives creators a linear writing surface while preserving structured `.nrstory` semantics.
_Avoid_: Free-form document editor, unparsed screenplay text, lossy graph export

**Script Conversion**:
The assisted transformation of a writer-provided script document, such as Word, Excel, or plain text, into a structured `.nrstory` file.
_Avoid_: Full script generation, runtime import

**Story Project Import Package**:
A conversion output package that can be imported into a GitHub-backed Story Project, containing generated story files and review notes rather than only a standalone `.nrstory` file.
_Avoid_: Single generated story file

**Conversion Profile**:
A project-level guide for Script Conversion that captures naming rules, cast assumptions, branch-recognition rules, and forbidden inference for a specific Story Project.
_Avoid_: Global AI settings, generated story output, runtime config

**Import Package Review**:
The product workflow for importing, validating, reviewing, correcting, and committing a Story Project Import Package before it becomes part of the authoritative Story Project repository.
_Avoid_: Directly trusting AI-generated files

**NarrRail-hosted AI Conversion Service**:
A future commercial AI service operated by NarrRail that converts writer-provided documents into Story Project Import Packages.
_Avoid_: First-stage core editor feature, user-provided API-key-only workflow
