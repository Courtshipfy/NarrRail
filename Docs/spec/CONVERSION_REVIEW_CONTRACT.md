# Conversion Review Contract

`conversion-notes.md` and optional `conversion-review.json` describe the review
surface created by Script Conversion before generated content is accepted into a
Story Project.

The contract is shared by:

- NarrRail Story Converter
- Story Project Import Package Review
- Project Review Queue

The converter may generate `.nrstory`, `.nroutline`, GlobalConfig, or conversion
profile files, but review metadata must stay outside those neutral story
formats.

## 1. Files

### `conversion-notes.md`

Human-readable notes. Required whenever the converter omits source content,
makes a conservative inference, finds ambiguity, or creates any item that needs
human review.

### `conversion-review.json`

Optional machine-readable metadata. Use it when review items should feed Project
Review Queue or future Import Package Review UI.

JSON never replaces Markdown. If both files exist, every JSON item should have a
matching human-readable entry in `conversion-notes.md`.

## 2. `conversion-notes.md` Template

```markdown
# Conversion Notes

## Source

- Source file:
- Conversion date:
- Project profile:
- Converter:

## Generated Outputs

| Path | Type | Notes |
|------|------|-------|
| Stories/chapter_01.nrstory | .nrstory | Draft conversion |

## Review Items

| ID | Severity | Issue Type | Source Location | Generated Target | Suggested Action |
|----|----------|------------|-----------------|------------------|------------------|
| review-001 | review | ambiguous-source | draft.docx lines 42-43 | Stories/chapter_01.nrstory / N_Chapter01_042 | Confirm speaker |

## Omitted Or Review-Only Source Content

| Source Location | Reason | Suggested Action |
|-----------------|--------|------------------|
| draft.docx line 88 | Author note, not story content | Confirm whether to keep as comment |

## Consistency With `conversion-review.json`

- JSON emitted: yes/no
- All JSON items represented above: yes/no
- Notes:
```

## 3. Review Item Vocabulary

### Severity

- `error`: Blocks accepting the import until resolved.
- `warning`: Should be reviewed before accepting the import.
- `review`: Needs human confirmation or records a conservative conversion choice.

Unknown severities normalize to `review`.

### Issue Type

Recommended values:

- `ambiguous-source`: source intent is unclear.
- `omitted-content`: content was left out of generated story files.
- `needs-confirmation`: generated output is plausible but needs human approval.
- `format-risk`: source content may require a NarrRail format feature not yet represented.
- `generated-target`: generated node, edge, or field needs review.
- `source-only`: review item has no generated target.
- `profile-mismatch`: source conflicts with project profile rules.

Consumers should display unknown issue types as categories instead of rejecting
the document.

### Source Location

Use as many fields as apply:

```json
{
  "path": "drafts/chapter-01.docx",
  "sheet": "Scene Table",
  "row": 12,
  "column": "Speaker",
  "lineStart": 42,
  "lineEnd": 43,
  "section": "Act 1 / Station",
  "excerpt": "I never said the station was safe."
}
```

### Generated Target

Use generated target fields when the item maps to NarrRail output:

```json
{
  "kind": "nrstory",
  "path": "Stories/chapter_01.nrstory",
  "nodeId": "N_Chapter01_042",
  "edgeId": "",
  "field": "dialogue.speakerId",
  "lineIndex": 0,
  "choiceIndex": null,
  "conditionBranchIndex": null
}
```

Allowed `kind` values:

- `nrstory`
- `nroutline`
- `global-config`
- `conversion-profile`
- `package`
- `source-only`

## 4. `conversion-review.json` Shape

```json
{
  "schemaVersion": 1,
  "packageId": "import-2026-07-11",
  "source": {
    "path": "drafts/chapter-01.docx",
    "title": "Chapter 01 Draft"
  },
  "generatedOutputs": [
    {
      "path": "Stories/chapter_01.nrstory",
      "type": "nrstory"
    }
  ],
  "items": [
    {
      "id": "review-001",
      "severity": "review",
      "issueType": "ambiguous-source",
      "message": "The speaker for this line needs confirmation.",
      "sourceLocation": {
        "path": "drafts/chapter-01.docx",
        "lineStart": 42,
        "lineEnd": 43,
        "excerpt": "I never said the station was safe."
      },
      "generatedTarget": {
        "kind": "nrstory",
        "path": "Stories/chapter_01.nrstory",
        "nodeId": "N_Chapter01_042",
        "field": "dialogue.speakerId"
      },
      "suggestedAction": "Confirm the speaker before accepting the import."
    }
  ]
}
```

Required fields:

- `schemaVersion`
- `items`

Required item fields:

- `severity`
- `issueType`
- `message`
- `suggestedAction`

At least one of `sourceLocation` or `generatedTarget` should be present.

## 5. Project Review Queue Mapping

When `conversion-review.json` is loaded from an Import Package:

- `issueType` maps to Project Review Queue `category`.
- `message` maps to `message`.
- `generatedTarget.path` maps to `assetPath`.
- `generatedTarget.nodeId`, `edgeId`, `field`, `lineIndex`, `choiceIndex`, and
  `conditionBranchIndex` map to queue item `target`.
- Import Package id maps to `target.importPackageId`.
- `sourceLocation`, `generatedTarget`, and `suggestedAction` remain attached to
  the queue item for future UI.

Invalid JSON should create a review queue `error` item with category `parse`.

## 6. Consistency Rules

- Do not place review-only notes inside `.nrstory`, GlobalConfig, or
  `.nroutline` fields.
- Markdown is the source for human review; JSON is the structured mirror.
- JSON item IDs should match Markdown row IDs when both exist.
- If a generated target is unavailable, write a source-only item with enough
  source location detail for a reviewer to find the issue.
- If a converter makes a conservative choice, describe both the choice and why
  the alternative was not used.
