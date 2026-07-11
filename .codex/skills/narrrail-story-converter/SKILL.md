---
name: narrrail-story-converter
description: Use when converting writer manuscripts, screenplay drafts, outlines, spreadsheets, prose, or other story source files into NarrRail .nrstory YAML; use when asked to infer scenes, dialogue, choices, branches, or story flow for NarrRail; use when validating or revising AI-generated .nrstory files.
---

# NarrRail Story Converter

Convert arbitrary writer manuscripts into NarrRail `.nrstory` YAML.

This skill is intentionally not a fixed-format parser. Manuscripts may be prose,
tables, screenplay-style drafts, outlines, notes, or mixed formats. Use AI
judgment to infer structure, then validate the result.

## Workflow

1. Read the source manuscript.
   - For `.xlsx`, `.csv`, `.tsv`, extract rows/cells first and preserve column
     meaning.
   - For `.docx` or PDF, extract readable text before conversion.
   - For loose prose, infer sections and dialogue conservatively.
2. Read only the references needed:
   - `references/nrstory-format.md` for required YAML shape.
   - `references/conversion-policy.md` for inference and ambiguity policy.
   - `references/project-profile-template.md` when creating or using a project
     profile.
   - `Docs/spec/CONVERSION_REVIEW_CONTRACT.md` when writing conversion notes or
     review metadata inside this repository.
3. Generate a `.nrstory` file.
4. Generate a companion `conversion-notes.md` file when any source content is
   omitted, ambiguous, inferred conservatively, or treated as review-only.
   Follow the shared review-note contract in
   `Docs/spec/CONVERSION_REVIEW_CONTRACT.md` when available.
   - When review items should feed Project Review Queue or Import Package Review,
     also emit optional `conversion-review.json` using the same item IDs and
     vocabulary as the Markdown notes.
5. Validate the `.nrstory`.
   - Prefer the repository validator when working inside NarrRail:
     `NarrRailEditor/src/utils/yaml-importer.js` and
     `NarrRailEditor/src/utils/validation.js`.
   - If unavailable, do a structural validation: unique node IDs, entry exists,
     all edge endpoints exist, Choice targets and choice handles are consistent.
6. Revise until validation has no errors.

## Output Policy

- Write generated files under a user-requested path when provided.
- Otherwise use a local `out/` directory near the source or current project.
- Do not mix review notes, second-pass hints, or writer comments into the main
  story path unless the user asks for them.
- Keep `conversion-notes.md` human-readable without tooling.
- Keep optional `conversion-review.json` machine-readable and consistent with
  `conversion-notes.md`; do not place review-only metadata inside `.nrstory`,
  GlobalConfig, or `.nroutline`.
- Keep `.nrstory` compatible with the neutral NarrRail format contract and declared Story Consumer compatibility:
  - one line: `Dialogue`
  - multiple consecutive lines by the same speaker: `MultiDialogue`
  - explicit alternatives: `Choice` with matching `choice-N` edges
  - no `edges[].condition`

## Optional External-AI Packet

If the user wants a prompt packet for another AI model instead of direct
conversion, run:

```bash
node .codex/skills/narrrail-story-converter/scripts/prepare-ai-conversion.mjs <manuscript> <output.md> --profile <profile.md>
```

The packet combines the project profile, NarrRail conversion guide, and source
manuscript.
