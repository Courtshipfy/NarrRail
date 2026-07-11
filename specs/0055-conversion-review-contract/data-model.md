# Data Model: Conversion Review Contract

## Conversion Notes

Human-readable Markdown file named `conversion-notes.md`.

Fields represented in prose or tables:

- Source summary
- Generated outputs
- Review item list
- Omitted or review-only content
- Consistency note for optional JSON metadata

Validation rules:

- If `conversion-review.json` exists, every JSON item should have a matching Markdown entry.
- Markdown must be useful without JSON tooling.

## Conversion Review Document

Machine-readable JSON file named `conversion-review.json`.

Fields:

- `schemaVersion`: integer, currently `1`
- `packageId`: optional Import Package identifier
- `source`: optional source document summary
- `generatedOutputs`: optional generated file list
- `items`: array of Conversion Review Items

Validation rules:

- `items` must be an array.
- Unknown fields may be preserved or ignored by readers.
- Invalid JSON produces a review queue parse/error item.

## Conversion Review Item

One ambiguity, omission, validation concern, or human-review request.

Fields:

- `id`: optional stable identifier
- `severity`: `error`, `warning`, or `review`
- `issueType`: category such as `ambiguous-source`, `omitted-content`, `needs-confirmation`, `format-risk`, or `generated-target`
- `message`: human-readable summary
- `sourceLocation`: source manuscript location
- `generatedTarget`: generated NarrRail target
- `suggestedAction`: what a reviewer should do next
- `notes`: optional extra details

Validation rules:

- Missing or unknown `severity` normalizes to `review`.
- `issueType` maps to the Project Review Queue category.
- `generatedTarget.path` maps to `assetPath` when present.
- `generatedTarget.nodeId` / `edgeId` maps to queue item target fields.

## Source Location

Fields:

- `path`
- `sheet`
- `row`
- `column`
- `lineStart`
- `lineEnd`
- `section`
- `excerpt`

Validation rules:

- At least one field should be present when the issue originates in source material.

## Generated Target

Fields:

- `kind`: `nrstory`, `nroutline`, `global-config`, `conversion-profile`, `package`, or `source-only`
- `path`
- `nodeId`
- `edgeId`
- `field`
- `lineIndex`
- `choiceIndex`
- `conditionBranchIndex`

Validation rules:

- `path` should be project-relative or package-relative.
- Source-only issues may omit generated target fields.
